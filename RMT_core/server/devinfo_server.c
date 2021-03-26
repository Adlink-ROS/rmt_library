#include <stdlib.h>
#include <string.h>
/* Used by DDS */
#include "dds/dds.h"
#include "DeviceInfo.h"
#include "rmt_server.h" // TODO: we only want to use the structure device_info
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

typedef struct _dev_list {
    struct _dev_list *next;
    device_info *info;
} dev_list;
static dev_list *g_dev_head = NULL;
static uint32_t g_dev_num = 0;

static dds_entity_t g_domain = 0;
static dds_entity_t g_participant;
static dds_entity_t g_reader;

static char g_interface[40];

static int add_device(DeviceInfo_Msg *msg)
{
    // Check whether the device exist or not.
    dev_list *dev_ptr = g_dev_head;
    while (dev_ptr) {
        if (dev_ptr->info->deviceID == msg->deviceID)
            break;
        dev_ptr = dev_ptr->next;
    }
    // Add/Update the device
    dev_list *selected_dev;
    if (dev_ptr == NULL) {
        // The device doesn't exist. Add it.
        selected_dev = (dev_list *) malloc(sizeof(dev_list));
        selected_dev->next = g_dev_head;
        g_dev_head = selected_dev;
        g_dev_num++;
        selected_dev->info = (device_info *) malloc(sizeof(device_info));
    } else {
        // The device exist. Update it.
        selected_dev = dev_ptr;
    }
    selected_dev->info->deviceID = msg->deviceID;
    selected_dev->info->host = strdup(msg->host);
    selected_dev->info->ip = strdup(msg->ip);
    selected_dev->info->mac = strdup(msg->mac);
    selected_dev->info->model = strdup(msg->model);
    selected_dev->info->rmt_version = strdup(msg->rmt_version);
    
    return 0;
}

static void free_dev_list(dev_list *dev_ptr)
{
    if (dev_ptr) {
        free(dev_ptr->info->host);
        free(dev_ptr->info->ip);
        free(dev_ptr->info->mac);
        free(dev_ptr->info->model);
        free(dev_ptr->info->rmt_version);
        free(dev_ptr->info);
        free(dev_ptr);
    }
}

static int del_device(int32_t id)
{
    dev_list *dev_ptr = g_dev_head;
    if (dev_ptr == NULL) goto exit;
    // If the device is on the head of list
    if (dev_ptr->info->deviceID == id) {
        g_dev_head = dev_ptr->next;
        g_dev_num--;
        goto exit;
    }
    // Check if the device exist
    while (dev_ptr->next) {
        if (dev_ptr->next->info->deviceID == id) {
            dev_list *to_be_freed = dev_ptr->next;
            dev_ptr->next = dev_ptr->next->next;
            dev_ptr = to_be_freed;
            goto exit;
        }
        dev_ptr = dev_ptr->next;
    }

exit:
    free_dev_list(dev_ptr);
    return 0;
}

int devinfo_server_config(char *interface)
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

int devinfo_server_init(void)
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
    g_reader = dds_create_reader(g_participant, topic, qos, NULL);
    if (g_reader < 0) {
        DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-g_reader));
        ret = -1;
        goto exit; 
    }
    dds_delete_qos(qos);

    /* Waiting */
    dds_sleepfor(DDS_MSECS(1000));

exit:
    return ret;
}

int devinfo_server_update(void)
{
    dds_sample_info_t infos[MAX_SAMPLES];
    void *samples[MAX_SAMPLES];
    DeviceInfo_Msg *msg;
    dds_return_t rc;
    int ret = 0;

    samples[0] = DeviceInfo_Msg__alloc();

    while(true) {
        rc = dds_take(g_reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (rc < 0) {
            DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
            ret = -1;
            break;
        }

        /* Check if we read some data and it is valid. */
        if ((rc > 0) && (infos[0].valid_data)) {
            msg = (DeviceInfo_Msg *) samples[0];
            add_device(msg);
        } else {
            // If there is no other device
            break;
        }
    }

    DeviceInfo_Msg_free(samples[0], DDS_FREE_ALL);

    return ret;
}

int devinfo_server_create_list(device_info **dev, uint32_t *num)
{
    int ret = 0;

    ret = devinfo_server_update();
    if (ret != 0) {
        goto exit;
    }

    *num = g_dev_num;
    *dev = (device_info *) malloc(sizeof(device_info) * g_dev_num);
    if (*dev == NULL) {
        ret = -1;
        goto exit;
    }
    dev_list *dev_ptr = g_dev_head;
    for (int i = 0; i < g_dev_num; i++) {
        (*dev)[i] = *dev_ptr->info;
        dev_ptr = dev_ptr->next;
    }

exit:
    return ret;
}

int devinfo_server_free_list(device_info **dev)
{
    if (dev == NULL || *dev == NULL)
        return -1;
    free(*dev);
    *dev = NULL;
    return 0;
}

int devinfo_server_deinit(void)
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

    // Remove all device
    while (g_dev_head) {
        dev_list *dev_ptr = g_dev_head;
        g_dev_head = g_dev_head->next;
        free_dev_list(dev_ptr);
        g_dev_num--;
    }

    return ret;
}