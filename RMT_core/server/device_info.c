/* Used by DDS */
#include "dds/dds.h"
#include "DeviceInfo.h"

#define MAX_SAMPLES 1

static dds_entity_t participant;
static dds_entity_t reader;

int device_info_subscriber_init(void)
{
    dds_entity_t topic;
    dds_qos_t *qos;
    dds_return_t rc;
    int ret = 0;

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0) {
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
        ret = -1;
        goto exit; 
    }

    /* Create a Topic. */
    topic = dds_create_topic (participant, &DeviceInfo_Msg_desc, "DeviceInfo_Msg", NULL, NULL);
    if (topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));    
        ret = -1;
        goto exit; 
    }

    /* Create a reliable Reader. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    reader = dds_create_reader(participant, topic, qos, NULL);
    if (reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
        ret = -1;
        goto exit; 
    }
    dds_delete_qos(qos);

exit:
    return ret;
}

int list_device_info(void)
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    DeviceInfo_Msg *msg;
    dds_return_t rc;
    int ret = 0;

    samples[0] = DeviceInfo_Msg__alloc();

    while(true) {
        rc = dds_read(reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0) {
            DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
            ret = -1;
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            msg = (DeviceInfo_Msg*) samples[0];
            printf("ID: %d\n", msg->deviceID);
            printf("Model: %s\n", msg->model);
            printf("Host: %s\n", msg->host);
            printf("IP: %s\n", msg->ip);
            printf("MAC: %s\n", msg->mac);
            printf("RMT version: %s\n", msg->rmt_version);
            fflush (stdout);
            break;
        } else {
            dds_sleepfor(DDS_MSECS(20));
        }
    }

    DeviceInfo_Msg_free(samples[0], DDS_FREE_ALL);

    return ret;
}

int device_info_subscriber_deinit(void)
{
    dds_return_t rc;
    int ret = 0;

    rc = dds_delete(participant);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
        ret = -1;
    }

    return ret;
}