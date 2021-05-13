#include <stdlib.h>
#include <string.h>

#include "dds_transport.h"
#include "DeviceInfo.h"
#include "rmt_server.h" // RMT_TODO: we only want to use the structure device_info
#include "logger.h"

typedef struct _dev_list {
    struct _dev_list *next;
    device_info *info;
    transfer_status agent_transfer_status;
    transfer_result transfer_result;
    long internal_id;
} dev_list;
static dev_list *g_dev_head = NULL;
static uint32_t g_dev_num = 0;

static int add_device(void *msg, void *recv_buf, void *arg)
{
    DeviceInfo_Msg *devinfo_msg = (DeviceInfo_Msg *) msg;
    // Check whether the device exist or not.
    dev_list *dev_ptr = g_dev_head;

    while (dev_ptr) {
        if (dev_ptr->info->deviceID == devinfo_msg->deviceID) {
            break;
        }
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
    selected_dev->info->deviceID = devinfo_msg->deviceID;
    selected_dev->info->host = strdup(devinfo_msg->host);
    selected_dev->info->ip = strdup(devinfo_msg->ip);
    selected_dev->info->mac = strdup(devinfo_msg->mac);
    selected_dev->info->model = strdup(devinfo_msg->model);
    selected_dev->info->rmt_version = strdup(devinfo_msg->rmt_version);
    selected_dev->internal_id = (long) arg;

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

static int del_device(long internal_id)
{
    dev_list *dev_ptr = g_dev_head;

    if (dev_ptr == NULL) {
        goto exit;
    }

    // If the device is on the head of list
    if (dev_ptr->internal_id == internal_id) {
        g_dev_head = dev_ptr->next;
        g_dev_num--;
        goto exit;
    }
    // Check if the device exist
    while (dev_ptr->next) {
        if (dev_ptr->next->internal_id == internal_id) {
            dev_list *to_be_freed = dev_ptr->next;
            dev_ptr->next = dev_ptr->next->next;
            dev_ptr = to_be_freed;
            goto exit;
        }
        dev_ptr = dev_ptr->next;
    }

exit:
    if (dev_ptr) {
        RMT_WARN("Lost device ID: %ld\n", dev_ptr->info->deviceID);
    }
    free_dev_list(dev_ptr);
    return 0;
}

int devinfo_server_del_device_callback(long internal_id)
{
    RMT_WARN("Some deviecs are lost.\n");
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

int devinfo_server_free_list(device_info *dev)
{
    if (dev == NULL) {
        return -1;
    }
    free(dev);
    return 0;
}

int devinfo_server_deinit(void)
{
    // Remove all device
    while (g_dev_head) {
        dev_list *dev_ptr = g_dev_head;
        g_dev_head = g_dev_head->next;
        free_dev_list(dev_ptr);
        g_dev_num--;
    }
}

int devinfo_server_set_status_by_id(int id, transfer_status dev_status, transfer_result dev_result)
{
    dev_list *dev_ptr = g_dev_head;

    for (int i = 0; i < g_dev_num; i++) {
        if (id == dev_ptr->info->deviceID) {
            dev_ptr->agent_transfer_status = dev_status;
            dev_ptr->transfer_result = dev_result;
            break;
        }
        dev_ptr = dev_ptr->next;
    }
}

int devinfo_server_get_status_by_id(int id, transfer_status *dev_status, transfer_result *dev_result)
{
    dev_list *dev_ptr = g_dev_head;

    for (int i = 0; i < g_dev_num; i++) {
        if (id == dev_ptr->info->deviceID) {
            *dev_status = dev_ptr->agent_transfer_status;
            *dev_result = dev_ptr->transfer_result;
            break;
        }
        dev_ptr = dev_ptr->next;
    }
}
