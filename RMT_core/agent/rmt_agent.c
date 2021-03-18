#include "dds/dds.h"
#include "DeviceInfo.h"
#include "rmt_agent.h"

void rmt_agent_init(void)
{
    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t writer;
    DeviceInfo_Msg msg;
    dds_return_t rc;
    uint32_t status = 0;
    dds_qos_t *qos;

    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0)
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

    /* Create a Topic. */
    topic = dds_create_topic(participant, &DeviceInfo_Msg_desc, "DeviceInfo_Msg", NULL, NULL);
    if (topic < 0)
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
    
    /* Create a Writer. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    writer = dds_create_writer(participant, topic, qos, NULL);
    if (writer < 0)
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));
    dds_delete_qos(qos);
    
    /* Create message */
    msg.deviceID = 1;
    msg.model = "ROScube-I";
    msg.host = "myhost";
    msg.ip = "1.2.3.4";
    msg.mac = "00:11:22:33:44:55";
    msg.rmt_version = "0.9.0";

    dds_sleepfor(DDS_MSECS(1000)); // Wait for data ready
    rc = dds_write(writer, &msg);
    if (rc != DDS_RETCODE_OK)
        DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));

    while (1) {
        dds_sleepfor(DDS_MSECS(1000));
    }

    /* Deleting the participant will delete all its children recursively as well. */
    rc = dds_delete(participant);
    if (rc != DDS_RETCODE_OK)
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

    return;
}