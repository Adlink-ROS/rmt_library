#include <stdlib.h>  // used by rand()
#include <time.h>    // used by time()
#include "DeviceInfo.h"
#include "dds/dds.h"
#include "config.h"

#define TOPIC_DEVICE_INFO "DeviceInfo_Msg"

static dds_entity_t g_participant;
static dds_entity_t g_writer;
static DeviceInfo_Msg g_msg;
static int g_info_change = 1;

static int device_info_publisher_update(void);

static void get_device_info(void)
{
    /* TODO: Need to get real information data */
    srand(time(NULL));
    g_msg.deviceID = rand(); // Just for test.
    g_msg.model = "ROScube-I";
    g_msg.host = "myhost";
    g_msg.ip = "1.2.3.4";
    g_msg.mac = "00:11:22:33:44:55";
    g_msg.rmt_version = PROJECT_VERSION;
}

int devinfo_agent_init(void)
{
    dds_entity_t topic;
    dds_return_t rc;
    dds_qos_t *qos;
    int ret = 0;

    /* Create a Participant */
    g_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
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

    return ret;
}