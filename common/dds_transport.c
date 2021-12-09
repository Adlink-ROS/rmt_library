#include <stdlib.h>
#include <string.h>
#include "dds/dds.h"
#include "dds_transport.h"
#include "DeviceInfo.h"
#include "DataInfo.h"
#include "logger.h"
#include "rmt_config.h"
#ifdef SUPPORT_ZENOH
 #include "far_dds_bridge_msgs.h"
#endif /*SUPPORT_ZENOH*/

#define TOPIC_DEVICE_INFO      "DeviceInfo_Msg"
#define TOPIC_PAIR_DATA_REQ    "DataReq_Msg"
#define TOPIC_PAIR_DATA_REPLY  "DataReply_Msg"
/*
 * The reason we need to disable multiple recv thread is that CycloneDDS will hang if we call dds_delete to delete the domain
 * This is because the recv thread need to wait for sending packets, but sending packets will not be available while disableing interface
 */
#define DDS_CONFIG "<CycloneDDS>" \
                   "  <Domain id=\"any\">" \
                   "    <General>" \
                   "      <NetworkInterfaceAddress>%s</NetworkInterfaceAddress>" \
                   "    </General>" \
                   "    <Internal>" \
                   "      <MultipleReceiveThreads>false</MultipleReceiveThreads>" \
                   "    </Internal>" \
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
static int g_domain_id = 0;
static unsigned int g_participant_num = 0;
typedef int (*device_delete_fptr)(uint64_t);
static device_delete_fptr g_device_delete_callback = NULL;
typedef struct dds_transport {
    dds_entity_t participant;
    dds_comm_pair pairs[PAIR_TOTAL];
} dds_transport;

int dds_transport_config_init(char *interface, int domain_id)
{
    char dds_config[2048];
    int ret = 0;

    sprintf(dds_config, DDS_CONFIG, interface);
    g_domain_id = domain_id;
    g_domain = dds_create_domain(g_domain_id, dds_config);
    // DDS_RETCODE_PRECONDITION_NOT_MET means the domain already exists
    if ((g_domain < 0) && (g_domain != DDS_RETCODE_PRECONDITION_NOT_MET)) {
        ret = -1;
    }

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
    transport->participant = dds_create_participant(g_domain_id, NULL, NULL);
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
    if ((ret == -1) && (transport != NULL)) {
        dds_delete(transport->participant);
        free(transport);
    }
    return transport;
}

static void callback_liveliness_changed(dds_entity_t rd, const dds_liveliness_changed_status_t status, void *arg)
{
    rd = rd;
    arg = arg;
    if (status.not_alive_count) {
        if (g_device_delete_callback) {
            g_device_delete_callback((long) status.last_publication_handle);
        }
    }
}

void callback_publication_matched(dds_entity_t writer, const dds_publication_matched_status_t status, void* arg)
{
    writer = writer;
    arg = arg;
    if (status.current_count_change > 0) {
        RMT_LOG("Detect RMT server.\n");
        RMT_LOG("Current RMT server number: %d\n", status.current_count);
    } else if (status.current_count_change < 0) {
        RMT_LOG("Lost RMT server.\n");
        RMT_LOG("Current RMT server number: %d\n", status.current_count);
    }
}

void callback_subscription_matched(dds_entity_t reader, const dds_subscription_matched_status_t status, void* arg)
{
    reader = reader;
    arg = arg;
    if (status.current_count_change < 0) {
        if (g_device_delete_callback) {
            g_device_delete_callback((long) status.last_publication_handle);
        }
    }
}

#ifdef SUPPORT_ZENOH
typedef int (*robot_id_delete_fptr)(char *);
static robot_id_delete_fptr g_robot_id_delete_callback = NULL;

void set_robot_id_delete_callback(int (*robot_id_delete_callback)(char *))
{
    g_robot_id_delete_callback = robot_id_delete_callback;
}

void callback_data_available(dds_entity_t reader, void* arg)
{
    dds_return_t rc;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t infos[MAX_SAMPLES];

    reader = reader;
    arg = arg;

    samples[0] = far_dds_bridge_msgs_msg_QosEvent__alloc();

    while (true) {
        rc = dds_take(reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0) {
            DDS_FATAL("dds_take: %s\n", dds_strretcode(-rc));
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            // Get data successfully
            far_dds_bridge_msgs_msg_QosEvent *msg;
            msg = (far_dds_bridge_msgs_msg_QosEvent*) samples[0];
            //printf("topic: %s, robot_id: %s, qos_event: %u\n", msg->topic, msg->robot_id, msg->qos_event);
            if (g_robot_id_delete_callback) {
                g_robot_id_delete_callback(msg->robot_id);
            }
        } else {
            // If there is no reply data
            break;
        }
    }

    far_dds_bridge_msgs_msg_QosEvent_free(samples[0], DDS_FREE_ALL);

    return;
}

#endif /*SUPPORT_ZENOH*/

struct dds_transport *dds_transport_server_init(int (*dev_delete_callback)(uint64_t))
{
    dds_transport *transport;
    dds_listener_t *listener;
    int ret = 0;

    transport = dds_transport_init();
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

#ifdef SUPPORT_ZENOH
    if (g_rmt_cfg.support_zenoh) {
        dds_entity_t zenoh_topic = dds_create_topic(transport->participant, &far_dds_bridge_msgs_msg_QosEvent_desc, "rt/qos_event", NULL, NULL);
        if (zenoh_topic < 0) {
            DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-zenoh_topic));
            ret = -1;
            goto exit;
        }
        dds_listener_t *zenoh_listener;
        zenoh_listener = dds_create_listener(NULL);
        dds_lset_data_available(zenoh_listener, callback_data_available);
        dds_qos_t *zenoh_qos = dds_create_qos();
        dds_qset_reliability(zenoh_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
        dds_entity_t zenoh_reader = dds_create_reader(transport->participant, zenoh_topic, zenoh_qos, zenoh_listener);
        if (zenoh_reader < 0) {
            DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-zenoh_reader));
            ret = -1;
            goto exit;
        }
        dds_delete_qos(zenoh_qos);
        dds_delete_listener(zenoh_listener);
    }
#endif /*SUPPORT_ZENOH*/

    /* Create a devinfo Reader. */
    listener = dds_create_listener(NULL);
    g_device_delete_callback = dev_delete_callback;
    dds_lset_liveliness_changed(listener, callback_liveliness_changed);
    dds_lset_subscription_matched(listener, callback_subscription_matched);
    dds_qos_t *devinfo_qos = dds_create_qos();
    dds_qset_reliability(devinfo_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(devinfo_qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    dds_qset_liveliness(devinfo_qos, DDS_LIVELINESS_AUTOMATIC, DDS_SECS(g_rmt_cfg.keepalive_time));
    transport->pairs[PAIR_DEV_INFO].reader = dds_create_reader(transport->participant, transport->pairs[PAIR_DEV_INFO].topic, devinfo_qos, listener);
    if (transport->pairs[PAIR_DEV_INFO].reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-transport->pairs[PAIR_DEV_INFO].reader));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(devinfo_qos);
    dds_delete_listener(listener);

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
    // Keep all history for reader, or we will miss some packets from agent.
    dds_qset_history(datainfo_qos, DDS_HISTORY_KEEP_ALL, 0);
    // Make sure the reply data only valid in 60 sec. This avoid too much old data which is not mine.
    dds_qset_lifespan(datainfo_qos, DDS_SECS(60));
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
    if ((ret == -1) && (transport != NULL)) {
        dds_delete(transport->participant);
        free(transport);
    }
    return transport;
}

struct dds_transport *dds_transport_agent_init(void)
{
    dds_transport *transport;
    dds_listener_t *listener;
    int ret = 0;

    transport = dds_transport_init();
    if (transport == NULL) {
        ret = -1;
        goto exit;
    }

    /* Create a devinfo Writer. */
    dds_qos_t *devinfo_qos = dds_create_qos();
    listener = dds_create_listener(NULL);
    dds_lset_publication_matched(listener, callback_publication_matched);
    dds_qset_reliability(devinfo_qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(devinfo_qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    dds_qset_liveliness(devinfo_qos, DDS_LIVELINESS_AUTOMATIC, DDS_SECS(g_rmt_cfg.keepalive_time));
    transport->pairs[PAIR_DEV_INFO].writer = dds_create_writer(transport->participant, transport->pairs[PAIR_DEV_INFO].topic, devinfo_qos, listener);
    if (transport->pairs[PAIR_DEV_INFO].writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-transport->pairs[PAIR_DEV_INFO].writer));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(devinfo_qos);
    dds_delete_listener(listener);

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
    if ((ret == -1) && (transport != NULL)) {
        dds_delete(transport->participant);
        free(transport);
    }
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

int dds_transport_try_recv(PAIR_KIND kind, struct dds_transport *transport, int (*func)(void *, void *, void *), void *arg)
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    dds_return_t rc;
    int ret = 0;

    samples[0] = dds_alloc(transport->pairs[kind].size);

    while (true) {
        rc = dds_take(transport->pairs[kind].reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0) {
            DDS_FATAL("dds_take: %s\n", dds_strretcode(-rc));
            ret = -1;
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            if (kind == PAIR_DEV_INFO) {
                func(samples[0], arg, (void *)infos[0].publication_handle);
            } else {
                func(samples[0], arg, NULL);
            }
        } else {
            // If there is no reply data
            break;
        }
    }

    dds_sample_free(samples[0], transport->pairs[kind].desc, DDS_FREE_ALL);

    return ret;
}

int dds_transport_try_recv_instance(void *instance, PAIR_KIND kind, struct dds_transport *transport, int (*func)(void *, void *, void *), void *arg)
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    dds_return_t rc;
    int ret = 0;
    dds_instance_handle_t hdl;

    samples[0] = dds_alloc(transport->pairs[kind].size);

    hdl = dds_lookup_instance(transport->pairs[kind].reader, instance);
    while (true) {
        rc = dds_take_instance(transport->pairs[kind].reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES, hdl);
        if (rc < 0) {
            // If there is no agent, it'll return DDS_RETCODE_PRECONDITION_NOT_MET. We should ignore this.
            if (rc != DDS_RETCODE_PRECONDITION_NOT_MET) {
                DDS_FATAL("dds_take_instance: %s\n", dds_strretcode(-rc));
            }
            ret = -1;
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            func(samples[0], arg, NULL);
        } else {
            // If there is no reply data
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
    if ((g_participant_num == 0) && (g_domain > 0)) {
        dds_delete(g_domain);
        g_domain = 0;
    }
    if (transport) {
        free(transport);
    }

    return ret;
}
