#include <stdlib.h>
#include <string.h>
#include "dds/dds.h"
#include "dds_transport.h"
#include "DeviceInfo.h"
#include "DataInfo.h"
#include "network.h"

#define DOMAIN_ID 0
#define TOPIC_DEVICE_INFO      "DeviceInfo_Msg"
#define TOPIC_PAIR_DATA_REQ    "DataReq_Msg"
#define TOPIC_PAIR_DATA_REPLY  "DataReply_Msg"
#define DDS_CONFIG "<CycloneDDS>" \
                   "  <Domain id=\"any\">" \
                   "    <General>" \
                   "      <NetworkInterfaceAddress>%s</NetworkInterfaceAddress>" \
                   "    </General>" \
                   "  </Domain>" \
                   "</CycloneDDS>"
#define MAX_SAMPLES 1

typedef struct dds_comm_pair {
    dds_entity_t topic;
    dds_entity_t reader;
    dds_entity_t writer;
    unsigned long size;
    const dds_topic_descriptor_t *desc;
} dds_comm_pair;

static dds_entity_t g_domain = 0;
static unsigned int g_participant_num = 0;
typedef struct dds_transport {
    dds_entity_t participant;
    dds_comm_pair pairs[PAIR_TOTAL];
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

    /* Create topic TOPIC_DEVICE_INFO */
    transport->pairs[PAIR_DEV_INFO].topic = dds_create_topic(transport->participant, &DeviceInfo_Msg_desc, TOPIC_DEVICE_INFO, NULL, NULL);
    if (transport->pairs[PAIR_DEV_INFO].topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-transport->pairs[PAIR_DEV_INFO].topic)); 
        ret = -1;
        goto exit;
    }
    transport->pairs[PAIR_DEV_INFO].desc = &DeviceInfo_Msg_desc;
    transport->pairs[PAIR_DEV_INFO].size = sizeof(DeviceInfo_Msg);

    /* Create topic TOPIC_PAIR_DATA_REQ */
    transport->pairs[PAIR_DATA_REQ].topic = dds_create_topic(transport->participant, &DataInfo_Request_desc, TOPIC_PAIR_DATA_REQ, NULL, NULL);
    if (transport->pairs[PAIR_DATA_REQ].topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-transport->pairs[PAIR_DATA_REQ].topic)); 
        ret = -1;
        goto exit;
    }
    transport->pairs[PAIR_DATA_REQ].desc = &DataInfo_Request_desc;
    transport->pairs[PAIR_DATA_REQ].size = sizeof(DataInfo_Request);

    /* Create topic TOPIC_PAIR_DATA_REPLY */
    transport->pairs[PAIR_DATA_REPLY].topic = dds_create_topic(transport->participant, &DataInfo_Reply_desc, TOPIC_PAIR_DATA_REPLY, NULL, NULL);
    if (transport->pairs[PAIR_DATA_REPLY].topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-transport->pairs[PAIR_DATA_REPLY].topic)); 
        ret = -1;
        goto exit;
    }
    transport->pairs[PAIR_DATA_REPLY].desc = &DataInfo_Reply_desc;
    transport->pairs[PAIR_DATA_REPLY].size = sizeof(DataInfo_Reply);

exit:
    return transport;
}

struct dds_transport *dds_transport_server_init(void)
{
    dds_transport *transport;
    int ret = 0;

    transport = dds_transport_init();
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

    /* Create a devinfo Reader. */
    dds_qos_t *devinfo_qos = dds_create_qos();
    dds_qset_reliability(devinfo_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(devinfo_qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    transport->pairs[PAIR_DEV_INFO].reader = dds_create_reader(transport->participant, transport->pairs[PAIR_DEV_INFO].topic, devinfo_qos, NULL);
    if (transport->pairs[PAIR_DEV_INFO].reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-transport->pairs[PAIR_DEV_INFO].reader));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(devinfo_qos);

    dds_qos_t *datainfo_qos = dds_create_qos();
    dds_qset_reliability(datainfo_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    /* Create a datainfo Writer. */
    transport->pairs[PAIR_DATA_REQ].writer = dds_create_writer(transport->participant, transport->pairs[PAIR_DATA_REQ].topic, datainfo_qos, NULL);
    if (transport->pairs[PAIR_DATA_REQ].writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-transport->pairs[PAIR_DATA_REQ].writer));
        ret = -1;
        goto exit;
    }
    /* Create a datainfo Reader*/
    transport->pairs[PAIR_DATA_REPLY].reader = dds_create_reader(transport->participant, transport->pairs[PAIR_DATA_REPLY].topic, datainfo_qos, NULL);
    if (transport->pairs[PAIR_DATA_REPLY].reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-transport->pairs[PAIR_DATA_REPLY].reader));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(datainfo_qos);

    /* Waiting */
    dds_sleepfor(DDS_MSECS(1000));

exit:
    return transport;
}

struct dds_transport *dds_transport_agent_init(void)
{
    dds_transport *transport;
    int ret = 0;

    transport = dds_transport_init();
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

    /* Create a devinfo Writer. */
    dds_qos_t *devinfo_qos = dds_create_qos();
    dds_qset_reliability(devinfo_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(devinfo_qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    transport->pairs[PAIR_DEV_INFO].writer = dds_create_writer(transport->participant, transport->pairs[PAIR_DEV_INFO].topic, devinfo_qos, NULL);
    if (transport->pairs[PAIR_DEV_INFO].writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-transport->pairs[PAIR_DEV_INFO].writer));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(devinfo_qos);

    dds_qos_t *datainfo_qos = dds_create_qos();
    dds_qset_reliability(datainfo_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    /* Create a datainfo Reader*/
    transport->pairs[PAIR_DATA_REQ].reader = dds_create_reader(transport->participant, transport->pairs[PAIR_DATA_REQ].topic, datainfo_qos, NULL);
    if (transport->pairs[PAIR_DATA_REQ].reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-transport->pairs[PAIR_DATA_REQ].reader));
        ret = -1;
        goto exit;
    }
    /* Create a datainfo Writer. */
    transport->pairs[PAIR_DATA_REPLY].writer = dds_create_writer(transport->participant, transport->pairs[PAIR_DATA_REPLY].topic, datainfo_qos, NULL);
    if (transport->pairs[PAIR_DATA_REPLY].writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-transport->pairs[PAIR_DATA_REPLY].writer));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(datainfo_qos);
    
exit:
    return transport;
}

int dds_transport_send(PAIR_KIND kind, struct dds_transport *transport, void *msg)
{
    int ret = 0;
    dds_return_t rc;
    dds_sleepfor(DDS_MSECS(1000)); // Wait for data ready
    rc = dds_write(transport->pairs[kind].writer, msg);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));
        ret = -1;
    }
    return ret;
}

int dds_transport_try_recv(PAIR_KIND kind, struct dds_transport *transport, int (*func)(void *))
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    dds_return_t rc;
    int ret = 0;

    samples[0] = dds_alloc(transport->pairs[kind].size);

    while(true) {
        rc = dds_take(transport->pairs[kind].reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0) {
            DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
            ret = -1;
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            func(samples[0]);
        } else {
            // If there is no other device
            break;
        }
    }

    dds_sample_free(samples[0], transport->pairs[kind].desc, DDS_FREE_ALL);

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