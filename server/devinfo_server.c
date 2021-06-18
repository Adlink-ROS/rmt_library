#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "dds_transport.h"
#include "DeviceInfo.h"
#include "rmt_server.h" // RMT_TODO: we only want to use the structure device_info
#include "logger.h"

typedef struct _dev_list {
    struct _dev_list *next;
    device_info *info;
    transfer_status agent_transfer_status;
    transfer_result transfer_result;
    uint64_t internal_id;
} dev_list;
static dev_list *g_dev_head = NULL;
static uint32_t g_dev_num = 0;
static pthread_mutex_t g_dev_mutex;

transfer_result empty_result;

static int add_device(void *msg, void *recv_buf, void *arg)
{
    DeviceInfo_Msg *devinfo_msg = (DeviceInfo_Msg *) msg;
    dev_list *dev_ptr;

    recv_buf = recv_buf;
    pthread_mutex_lock(&g_dev_mutex);
    // Check whether the device exist or not.
    dev_ptr = g_dev_head;
    while (dev_ptr) {
        if (dev_ptr->info->deviceID == devinfo_msg->deviceID) {
            break;
        }
        dev_ptr = dev_ptr->next;
    }
    // Add/Update the device
    dev_list *selected_dev;
    RMT_LOG("Found new device %lu\n", devinfo_msg->deviceID);
    if (dev_ptr == NULL) {
        // The device doesn't exist. Create a new one.
        selected_dev = (dev_list *) malloc(sizeof(dev_list));
        selected_dev->info = (device_info *) malloc(sizeof(device_info));
    } else {
        // The device exist. Update it.
        RMT_WARN("The device with same ID %lu already exists. Will be replaced.\n", devinfo_msg->deviceID);
        selected_dev = dev_ptr;
    }
    selected_dev->info->deviceID = devinfo_msg->deviceID;
    selected_dev->info->host = strdup(devinfo_msg->host);
    selected_dev->info->ip = strdup(devinfo_msg->ip);
    selected_dev->info->mac = strdup(devinfo_msg->mac);
    selected_dev->info->model = strdup(devinfo_msg->model);
    selected_dev->info->rmt_version = strdup(devinfo_msg->rmt_version);
    selected_dev->info->devinfo = strdup(devinfo_msg->devinfo);
    selected_dev->internal_id = (uint64_t) arg;

    // Add new device at the head if this is new device
    // Note: g_dev_num should ALWAYS put at the bottom. We should make sure selected_dev is ready before putting into linked list.
    //       Otherwise, user might get the wrong data.
    if (dev_ptr == NULL) {
        selected_dev->next = g_dev_head;
        g_dev_head = selected_dev;
        g_dev_num++;
    }
    pthread_mutex_unlock(&g_dev_mutex);

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
        free(dev_ptr->info->devinfo);
        free(dev_ptr->info);
        free(dev_ptr);
    }
}

static int del_device(uint64_t internal_id)
{
    dev_list *dev_ptr;
    dev_list *to_be_freed = NULL;

    pthread_mutex_lock(&g_dev_mutex);
    dev_ptr = g_dev_head;
    if (dev_ptr == NULL) {
        goto exit;
    }

    // If the device is on the head of list
    if (dev_ptr->internal_id == internal_id) {
        g_dev_head = dev_ptr->next;
        g_dev_num--;
        to_be_freed = dev_ptr;
        goto exit;
    }
    // Check if the device exist
    while (dev_ptr->next) {
        if (dev_ptr->next->internal_id == internal_id) {
            to_be_freed = dev_ptr->next;
            dev_ptr->next = dev_ptr->next->next;
            g_dev_num--;
            goto exit;
        }
        dev_ptr = dev_ptr->next;
    }

exit:
    pthread_mutex_unlock(&g_dev_mutex);
    // If the matched internal ID exist in our device list.
    if (to_be_freed) {
        RMT_WARN("Lost device ID %lu\n", to_be_freed->info->deviceID);
        free_dev_list(to_be_freed);
    }
    return 0;
}

int devinfo_server_del_device_callback(uint64_t internal_id)
{
    return del_device(internal_id);
}

int devinfo_server_update(struct dds_transport *transport)
{
    return dds_transport_try_recv(PAIR_DEV_INFO, transport, add_device, NULL);
}

int devinfo_server_create_list(struct dds_transport *transport, device_info **dev, uint32_t *num)
{
    int ret = 0;

    ret = devinfo_server_update(transport);
    if (ret != 0) {
        goto exit;
    }

    pthread_mutex_lock(&g_dev_mutex);
    *num = g_dev_num;
    *dev = (device_info *) malloc(sizeof(device_info) * g_dev_num);
    if (*dev == NULL) {
        ret = -1;
        goto exit_mutex;
    }
    dev_list *dev_ptr = g_dev_head;
    for (unsigned int i = 0; i < g_dev_num; i++) {
        (*dev)[i] = *dev_ptr->info;
        dev_ptr = dev_ptr->next;
    }

exit_mutex:
    // RMT_TODO: What will happen if the device is removed and user still can access the device list?
    //           I think they can't use the same memory.
    pthread_mutex_unlock(&g_dev_mutex);
exit:
    return ret;
}

int devinfo_server_free_list(device_info *dev)
{
    if (dev == NULL) {
        return -1;
    }
    free(dev);
    return 0;
}

void devinfo_server_init(void)
{
    g_dev_head = NULL;
    g_dev_num = 0;
    empty_result.result = 0;
    empty_result.file_len = 0;
    empty_result.pFile = NULL;
    pthread_mutex_init(&g_dev_mutex, NULL);
}

void devinfo_server_deinit(void)
{
    pthread_mutex_lock(&g_dev_mutex);
    // Remove all device
    while (g_dev_head) {
        dev_list *dev_ptr = g_dev_head;
        g_dev_head = g_dev_head->next;
        free_dev_list(dev_ptr);
        g_dev_num--;
    }
    pthread_mutex_unlock(&g_dev_mutex);
    // RMT_TODO: What if other function is called while calling deinit?
    pthread_mutex_destroy(&g_dev_mutex);
}

int devinfo_server_set_status_by_id(unsigned long id, transfer_status dev_status, transfer_result dev_result)
{
    int ret = -1;
    dev_list *dev_ptr;

    pthread_mutex_lock(&g_dev_mutex);
    dev_ptr = g_dev_head;
    for (unsigned int i = 0; i < g_dev_num; i++) {
        if (id == dev_ptr->info->deviceID) {
            dev_ptr->agent_transfer_status = dev_status;
            dev_ptr->transfer_result = dev_result;
            ret = 0;
            break;
        }
        dev_ptr = dev_ptr->next;
    }
    pthread_mutex_unlock(&g_dev_mutex);

    return ret;
}

int devinfo_server_get_status_by_id(unsigned long id, transfer_status *dev_status, transfer_result *dev_result)
{
    dev_list *dev_ptr;
    int found = 0;

    pthread_mutex_lock(&g_dev_mutex);
    dev_ptr = g_dev_head;
    for (unsigned int i = 0; i < g_dev_num; i++) {
        if (id == dev_ptr->info->deviceID) {
            *dev_status = dev_ptr->agent_transfer_status;
            *dev_result = dev_ptr->transfer_result;
            found = 1;
            break;
        }
        dev_ptr = dev_ptr->next;
    }
    pthread_mutex_unlock(&g_dev_mutex);

    if (!found) {
        *dev_status = STATUS_AGENT_ERROR;
        *dev_result = empty_result;
    }

    return 0;
}
