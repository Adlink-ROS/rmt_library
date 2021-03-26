#include <stdlib.h>  // used by rand()
#include <time.h>    // used by time()
#include <string.h>
#include "DeviceInfo.h"
#include "dds/dds.h"
#include "version.h"
#include "network.h"

#define DOMAIN_ID 0
#define TOPIC_DEVICE_INFO "DeviceInfo_Msg"
#define DDS_CONFIG "<CycloneDDS>" \
                   "  <Domain id=\"any\">" \
                   "    <General>" \
                   "      <NetworkInterfaceAddress>%s</NetworkInterfaceAddress>" \
                   "    </General>" \
                   "  </Domain>" \
                   "</CycloneDDS>"

static dds_entity_t g_domain;
static dds_entity_t g_participant;
static dds_entity_t g_writer;
static DeviceInfo_Msg g_msg;
static int g_info_change = 1;
typedef struct _device_info {
    char hostname[1024];
    char interface[40];
    char ip[40];
    char mac[20];
} device_info;
static device_info g_dev;

static int device_info_publisher_update(void);

static void get_device_info(void)
{
    device_info tmp_dev = g_dev;

    // Check hostname
    gethostname(tmp_dev.hostname, sizeof(tmp_dev.hostname));
    g_dev.hostname[sizeof(tmp_dev.hostname) - 1] = 0;
    // Get IP
    net_get_ip(tmp_dev.interface, tmp_dev.ip, sizeof(tmp_dev.ip));
    // Get MAC
    net_get_mac(tmp_dev.interface, tmp_dev.mac, sizeof(tmp_dev.mac));

    if (memcmp(&g_dev, &tmp_dev, sizeof(device_info)) != 0) {
        g_dev = tmp_dev;
        g_info_change = 1;
    }

    g_msg.model = "ROScube-I"; // TODO: get the correct model.
    g_msg.host = g_dev.hostname;
    g_msg.ip = g_dev.ip;
    g_msg.mac = g_dev.mac;
    g_msg.rmt_version = PROJECT_VERSION;
}

int devinfo_agent_config(char *interface, int id)
{
    int ret = 0;

    /* Parse ID */
    if (id == 0) {
        g_msg.deviceID = net_get_id_from_mac(g_dev.interface);
    } else {
        g_msg.deviceID = id;
    }

    /* 
     * First, use interface user assigns
     * If no, select the interface by ourselves
     * If fail, return error.
     */
    if (interface != NULL) {
        strcpy(g_dev.interface, interface);
    } else if (net_select_interface(g_dev.interface) < 0) {
        ret = -1;
        goto exit;
    }
exit:
    return ret;
}

int devinfo_agent_init(void)
{
    dds_entity_t topic;
    dds_return_t rc;
    dds_qos_t *qos;
    int ret = 0;
    char dds_config[2048];

    sprintf(dds_config, DDS_CONFIG, g_dev.interface);
    g_domain = dds_create_domain(DOMAIN_ID, dds_config);
    /* Create a Participant */
    g_participant = dds_create_participant(DOMAIN_ID, NULL, NULL);
    if (g_participant < 0) {
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-g_participant));
        ret = -1;
        goto exit;
    }

    /* Create a Topic. */
    topic = dds_create_topic(g_participant, &DeviceInfo_Msg_desc, TOPIC_DEVICE_INFO, NULL, NULL);
    if (topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
        ret = -1;
        goto exit;
    }
    
    /* Create a Writer. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    g_writer = dds_create_writer(g_participant, topic, qos, NULL);
    if (g_writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-g_writer));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(qos);
    
exit:
    return ret;
}

int devinfo_agent_update(void)
{
    dds_return_t rc;
    int ret = 0;

    get_device_info();

    /* If info mation changes */
    if (g_info_change) {
        dds_sleepfor(DDS_MSECS(1000)); // Wait for data ready
        rc = dds_write(g_writer, &g_msg);
        if (rc != DDS_RETCODE_OK) {
            DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));
            ret = -1;
        }
        g_info_change = 0;
    }

    return ret;
}

int devinfo_agent_deinit(void)
{
    dds_return_t rc;
    int ret = 0;

    /* Deleting the g_participant will delete all its children recursively as well. */
    rc = dds_delete(g_participant);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
        ret = -1;
    }
    /* Delete g_domain */
    dds_delete(g_domain);

    return ret;
}