#include "DeviceInfo.h"
#include "dds/dds.h"

static dds_entity_t participant;
static dds_entity_t writer;
static DeviceInfo_Msg msg;
static int info_change = 1;

static int device_info_publisher_update(void);

int update_device_info(void)
{
    /* TODO: Need to get real information data */
    msg.deviceID = 1;
    msg.model = "ROScube-I";
    msg.host = "myhost";
    msg.ip = "1.2.3.4";
    msg.mac = "00:11:22:33:44:55";
    msg.rmt_version = "0.9.0";

    return device_info_publisher_update();
}

int device_info_publisher_init(void)
{
    dds_entity_t topic;
    dds_return_t rc;
    dds_qos_t *qos;
    int ret = 0;

    /* Create a Participant */
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0) {
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
        ret = -1;
        goto exit;
    }

    /* Create a Topic. */
    topic = dds_create_topic(participant, &DeviceInfo_Msg_desc, "DeviceInfo_Msg", NULL, NULL);
    if (topic < 0) {
        DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
        ret = -1;
        goto exit;
    }
    
    /* Create a Writer. */
    qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    writer = dds_create_writer(participant, topic, qos, NULL);
    if (writer < 0) {
        DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));
        ret = -1;
        goto exit;
    }
    dds_delete_qos(qos);
    
exit:
    return ret;
}

static int device_info_publisher_update(void)
{
    dds_return_t rc;
    int ret = 0;

    /* If info mation changes */
    if (info_change) {
        dds_sleepfor(DDS_MSECS(1000)); // Wait for data ready
        rc = dds_write(writer, &msg);
        if (rc != DDS_RETCODE_OK) {
            DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));
            ret = -1;
        }
    }

    return ret;
}

int device_info_publisher_deinit(void)
{
    dds_return_t rc;
    int ret = 0;

    /* Deleting the participant will delete all its children recursively as well. */
    rc = dds_delete(participant);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
        ret = -1;
    }

    return ret;
}