#include <stdlib.h>
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
static unsigned int g_participant_num = 0;
typedef struct dds_transport {
    dds_entity_t participant;
    dds_entity_t devinfo_reader;
    dds_entity_t devinfo_writer;
    dds_entity_t devinfo_topic;
} dds_transport;

int dds_transport_config_init(char *interface)
{
    char dds_config[2048];
    char selected_interface[40];

    int ret = 0;
    if (interface != NULL) {
        strcpy(selected_interface, interface);
    } else if (net_select_interface(selected_interface) < 0) {
        ret = -1;
        goto exit;
    }

    sprintf(dds_config, DDS_CONFIG, selected_interface);
    g_domain = dds_create_domain(DOMAIN_ID, dds_config);
exit:
    return ret;
}

static struct dds_transport *dds_transport_init(void)
{
    int ret = 0;
    dds_transport *transport = (dds_transport *) malloc(sizeof(dds_transport));
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

    /* Create a Participant. */
    transport->participant = dds_create_participant(DOMAIN_ID, NULL, NULL);
    if (transport->participant < 0) {
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-transport->participant));
        ret = -1;
        goto exit; 
    }
    g_participant_num++;

    /* Create a Topic. */
    transport->devinfo_topic = dds_create_topic(transport->participant, &DeviceInfo_Msg_desc, TOPIC_DEVICE_INFO, NULL, NULL);
    if (transport->devinfo_topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-transport->devinfo_topic));    
        ret = -1;
        goto exit; 
    }

exit:
    return transport;
}

struct dds_transport *dds_transport_server_init(void)
{
    dds_transport *transport;
    dds_qos_t *qos;
    int ret = 0;

    transport = dds_transport_init();
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

    /* Create a reliable Reader. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    transport->devinfo_reader = dds_create_reader(transport->participant, transport->devinfo_topic, qos, NULL);
    if (transport->devinfo_reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-transport->devinfo_reader));
        ret = -1;
        goto exit; 
    }
    dds_delete_qos(qos);

    /* Waiting */
    dds_sleepfor(DDS_MSECS(1000));

exit:
    return transport;
}

struct dds_transport *dds_transport_agent_init(void)
{
    dds_transport *transport;
    dds_qos_t *qos;
    int ret = 0;

    transport = dds_transport_init();
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

    /* Create a Writer. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    transport->devinfo_writer = dds_create_writer(transport->participant, transport->devinfo_topic, qos, NULL);
    if (transport->devinfo_writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-transport->devinfo_writer));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(qos);
    
exit:
    return transport;
}

int dds_transport_send_devinfo(struct dds_transport *transport, struct DeviceInfo_Msg *msg)
{
    int ret = 0;
    dds_return_t rc;
    dds_sleepfor(DDS_MSECS(1000)); // Wait for data ready
    rc = dds_write(transport->devinfo_writer, msg);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));
        ret = -1;
    }
    return ret;
}

int dds_transport_try_get_devinfo(struct dds_transport *transport, int (*func)(struct DeviceInfo_Msg *))
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    DeviceInfo_Msg *msg;
    dds_return_t rc;
    int ret = 0;

    samples[0] = DeviceInfo_Msg__alloc();

    while(true) {
        rc = dds_take(transport->devinfo_reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
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

int dds_transport_deinit(struct dds_transport *transport)
{
    dds_return_t rc;
    int ret = 0;

    /* Deleting the g_participant will delete all its children recursively as well. */
    rc = dds_delete(transport->participant);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
        ret = -1;
    }
    g_participant_num--;
    /* Delete g_domain */
    if (g_participant_num == 0 && g_domain > 0) {
        dds_delete(g_domain);
        g_domain = 0;
    }

    return ret;
}