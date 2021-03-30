#include <string.h>
#include "dds/dds.h"
#include "dds_transport.h"
#include "DeviceInfo.h"
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
#define MAX_SAMPLES 1

static dds_entity_t g_domain = 0;
static dds_entity_t g_participant;
static dds_entity_t g_devinfo_reader;

static char g_interface[40];

int dds_transport_domain_init(char *interface)
{
    char dds_config[2048];

    int ret = 0;
    if (interface != NULL) {
        strcpy(g_interface, interface);
    } else if (net_select_interface(g_interface) < 0) {
        ret = -1;
        goto exit;
    }

    sprintf(dds_config, DDS_CONFIG, g_interface);
    g_domain = dds_create_domain(DOMAIN_ID, dds_config);
exit:
    return ret;
}

int dds_transport_init(void)
{
    dds_entity_t topic;
    dds_qos_t *qos;
    dds_return_t rc;
    int ret = 0;

    /* Create a Participant. */
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

    /* Create a reliable Reader. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    g_devinfo_reader = dds_create_reader(g_participant, topic, qos, NULL);
    if (g_devinfo_reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-g_devinfo_reader));
        ret = -1;
        goto exit; 
    }
    dds_delete_qos(qos);

    /* Waiting */
    dds_sleepfor(DDS_MSECS(1000));

exit:
    return ret;
}

int dds_transport_try_get_devinfo(int (*func)(struct DeviceInfo_Msg *))
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    DeviceInfo_Msg *msg;
    dds_return_t rc;
    int ret = 0;

    samples[0] = DeviceInfo_Msg__alloc();

    while(true) {
        rc = dds_take(g_devinfo_reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0) {
            DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
            ret = -1;
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            msg = (DeviceInfo_Msg *) samples[0];
            func(msg);
        } else {
            // If there is no other device
            break;
        }
    }

    DeviceInfo_Msg_free(samples[0], DDS_FREE_ALL);

    return ret;
}

int dds_transport_deinit(void)
{
    dds_return_t rc;
    int ret = 0;

    rc = dds_delete(g_participant);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
        ret = -1;
    }
    /* Delete g_domain */
    if (g_domain > 0) {
        dds_delete(g_domain);
        g_domain = 0;
    }

    return ret;
}