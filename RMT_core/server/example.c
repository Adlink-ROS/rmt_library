#include <stdio.h>
#include "rmt_lib.h"
#include "dds/dds.h"
#include "DeviceInfo.h"

#define MAX_SAMPLES 1

int main(void)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader;
    DeviceInfo_Msg *msg;
    dds_qos_t *qos;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t infos[MAX_SAMPLES];
    dds_return_t rc;

    printf("RMT Library version is %s\n", rmt_lib_version());

    /* Create a Participant. */
    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0)
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

    /* Create a Topic. */
    topic = dds_create_topic (participant, &DeviceInfo_Msg_desc, "DeviceInfo_Msg", NULL, NULL);
    if (topic < 0)
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));    

    /* Create a reliable Reader. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    reader = dds_create_reader(participant, topic, qos, NULL);
    if (reader < 0)
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
    dds_delete_qos(qos);

    samples[0] = DeviceInfo_Msg__alloc();

    while(true) {
        rc = dds_read(reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0)
            DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));

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
            dds_sleepfor(DDS_MSECS (20));
        }
    }

    DeviceInfo_Msg_free(samples[0], DDS_FREE_ALL);

    rc = dds_delete(participant);
    if (rc != DDS_RETCODE_OK)
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

    return 0;
}