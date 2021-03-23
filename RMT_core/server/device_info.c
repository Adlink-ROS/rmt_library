#include <stdlib.h>
#include <string.h>
/* Used by DDS */
#include "dds/dds.h"
#include "DeviceInfo.h"
#include "rmt_server.h" // TODO: we only want to use the structure device_info

#define TOPIC_DEVICE_INFO "DeviceInfo_Msg"

#define MAX_SAMPLES 1

typedef struct _dev_list {
    struct _dev_list *next;
    device_info *info;
} dev_list;
static dev_list *dev_head = NULL;
static uint32_t dev_num = 0;

static dds_entity_t participant;
static dds_entity_t reader;

static int add_device(DeviceInfo_Msg *msg)
{
    // Check whether the device exist or not.
    dev_list *ptr = dev_head;
    while (ptr) {
        if (ptr->info->deviceID == msg->deviceID)
            break;
        ptr = ptr->next;
    }
    // Add/Update the device
    dev_list *selected_dev;
    if (ptr == NULL) {
        // The device doesn't exist. Add it.
        selected_dev = (dev_list *) malloc(sizeof(dev_list));
        selected_dev->next = dev_head;
        dev_head = selected_dev;
        dev_num++;
        selected_dev->info = (device_info *) malloc(sizeof(device_info));
    } else {
        // The device exist. Update it.
        selected_dev = ptr;
    }
    selected_dev->info->deviceID = msg->deviceID;
    selected_dev->info->host = strdup(msg->host);
    selected_dev->info->ip = strdup(msg->ip);
    selected_dev->info->mac = strdup(msg->mac);
    selected_dev->info->model = strdup(msg->model);
    selected_dev->info->rmt_version = strdup(msg->rmt_version);
    
    return 0;
}

static void free_dev_list(dev_list *ptr)
{
    if (ptr) {
        free(ptr->info->host);
        free(ptr->info->ip);
        free(ptr->info->mac);
        free(ptr->info->model);
        free(ptr->info->rmt_version);
        free(ptr);
    }
}

static int del_device(int32_t id)
{
    dev_list *ptr = dev_head;
    if (ptr == NULL) goto exit;
    // If the device is on the head of list
    if (ptr->info->deviceID == id) {
        dev_head = ptr->next;
        dev_num--;
        goto exit;
    }
    // Check if the device exist
    while (ptr->next) {
        if (ptr->next->info->deviceID == id) {
            dev_list *to_be_freed = ptr->next;
            ptr->next = ptr->next->next;
            ptr = to_be_freed;
            goto exit;
        }
        ptr = ptr->next;
    }

exit:
    free_dev_list(ptr);
    return 0;
}

int device_info_init(void)
{
    dds_entity_t topic;
    dds_qos_t *qos;
    dds_return_t rc;
    int ret = 0;

    /* Create a Participant. */
    participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    if (participant < 0) {
        DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
        ret = -1;
        goto exit; 
    }

    /* Create a Topic. */
    topic = dds_create_topic(participant, &DeviceInfo_Msg_desc, TOPIC_DEVICE_INFO, NULL, NULL);
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

int device_info_update(void)
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
            add_device(msg);
            break;
        } else {
            dds_sleepfor(DDS_MSECS(20));
        }
    }

    DeviceInfo_Msg_free(samples[0], DDS_FREE_ALL);

    return ret;
}

int device_info_create_list(device_info **dev, uint32_t *num)
{
    int ret = 0;

    ret = device_info_update();
    if (ret != 0) {
        goto exit;
    }

    *num = dev_num;
    *dev = (device_info *) malloc(sizeof(device_info) * dev_num);
    if (*dev == NULL) {
        ret = -1;
        goto exit;
    }
    dev_list *ptr = dev_head;
    for (int i = 0; i < dev_num; i++) {
        (*dev)[i] = *ptr->info;
        ptr = ptr->next;
    }

exit:
    return ret;
}

int device_info_free_list(device_info **dev)
{
    if (*dev == NULL)
        return -1;
    free(*dev);
    *dev = NULL;
    return 0;
}

int device_info_deinit(void)
{
    dds_return_t rc;
    int ret = 0;

    rc = dds_delete(participant);
    if (rc != DDS_RETCODE_OK) {
        DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
        ret = -1;
    }

    // Remove all device
    while (dev_head) {
        dev_list *ptr = dev_head;
        dev_head = dev_head->next;
        free_dev_list(ptr);
        dev_num--;
    }

    return ret;
}