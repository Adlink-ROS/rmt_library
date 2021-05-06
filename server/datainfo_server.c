#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "dds_transport.h"
#include "rmt_server.h"
#include "devinfo_server.h"
#include "DataInfo.h"
#include "logger.h"

// RMT_TODO: This should be configurable.
#define DEFAULT_TIMEOUT 3

// RMT_TODO: To support simultaneous access, should not use one global variable.
static DataInfo_Request g_msg;
static data_info *g_reply_list;
static int g_reply_num;

static int recv_reply(void *msg, void *arg)
{
    DataInfo_Reply *datainfo_msg = (DataInfo_Reply *) msg;

    // Check whether this reply is for me or not.
    if ((datainfo_msg->type != g_msg.type) || (datainfo_msg->random_seq != g_msg.random_seq)) {
        return -1;
    }

    RMT_LOG("Receive device ID: %ld\n", datainfo_msg->deviceID);
    g_reply_list[g_reply_num].deviceID = datainfo_msg->deviceID;
    strncpy(g_reply_list[g_reply_num].value_list, datainfo_msg->msg, CONFIG_KEY_STR_LEN);
    g_reply_num++;
    return 0;
}

static int recv_file_transfer_reply(void *msg, void *arg)
{
    DataInfo_Reply *datainfo_msg = (DataInfo_Reply *) msg;
    transfer_status status;

    // Check whether this reply is for me or not.
    if ((datainfo_msg->type != g_msg.type) || (datainfo_msg->random_seq != g_msg.random_seq)) {
        return -1;
    }

    RMT_LOG("Receive device ID: %ld\n", datainfo_msg->deviceID);
    RMT_LOG("Receive result: %s\n", datainfo_msg->msg);
    transfer_result file_result;
    file_result.result = atoi(datainfo_msg->msg);
    if (datainfo_msg->type == DataInfo_IMPORT) {
        file_result.pFile = NULL;
        file_result.file_len = 0;
    } else if (datainfo_msg->type == DataInfo_EXPORT) {
        file_result.pFile = malloc(sizeof(uint8_t) * datainfo_msg->binary._length);
        if (file_result.pFile) {
            memcpy(file_result.pFile, datainfo_msg->binary._buffer, datainfo_msg->binary._length);
            file_result.file_len = datainfo_msg->binary._length;
            status = TRANSFER_DONE;
        } else {
            status = SERVER_ERROR;
        }
    }
    devinfo_server_set_status_by_id(datainfo_msg->deviceID, status, file_result);
    g_reply_num++;
    return 0;
}

data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, int id_num, char *key_list, int *info_num)
{
    // clean the reply queue
    g_reply_list = (data_info *) malloc(sizeof(data_info) * id_num);
    g_reply_num = 0;

    // Build up request message
    g_msg.id_list._maximum = g_msg.id_list._length = id_num;
    g_msg.id_list._buffer = id_list;
    g_msg.msg = key_list;
    g_msg.type = DataInfo_GET;
    srand(time(NULL));
    g_msg.random_seq = rand();
    g_msg.binary._buffer = NULL;
    g_msg.binary._maximum = g_msg.binary._length = 0;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (g_reply_num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("get info timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, g_reply_num);
            break;
        }
        dds_transport_try_recv(PAIR_DATA_REPLY, transport, recv_reply);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }
    *info_num = g_reply_num;

    return g_reply_list;
}

int datainfo_server_free_info(data_info* info_list)
{
    free(info_list);
    return 0;
}

data_info* datainfo_server_set_info(struct dds_transport *transport, data_info *dev_list, int dev_num, int *info_num)
{
    // clean the reply queue
    g_reply_list = (data_info *) malloc(sizeof(data_info) * dev_num);
    g_reply_num = 0;

    // Build up request message
    int id_num = dev_num;
    unsigned long *id_list = malloc(sizeof(unsigned long) * id_num);
    char *buffer = malloc(1024 * id_num);

    buffer[0] = 0;
    for (int i = 0; i < dev_num; i++) {
        id_list[i] = dev_list[i].deviceID;
        strcat(buffer, dev_list[i].value_list);
        strcat(buffer, "|");
    }
    g_msg.id_list._maximum = g_msg.id_list._length = id_num;
    g_msg.id_list._buffer = id_list;
    g_msg.msg = buffer;
    g_msg.type = DataInfo_SET;
    srand(time(NULL));
    g_msg.random_seq = rand();
    g_msg.binary._buffer = NULL;
    g_msg.binary._maximum = g_msg.binary._length = 0;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (g_reply_num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("set info timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, g_reply_num);
            break;
        }
        dds_transport_try_recv(PAIR_DATA_REPLY, transport, recv_reply);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }
    *info_num = g_reply_num;

    free(id_list);
    free(buffer);

    return g_reply_list;
}

data_info* datainfo_server_set_info_with_same_value(struct dds_transport *transport, unsigned long *id_list, int id_num, char *value_list, int *info_num)
{
    // clean the reply queue
    g_reply_list = (data_info *) malloc(sizeof(data_info) * id_num);
    g_reply_num = 0;

    // Build up request message
    g_msg.id_list._maximum = g_msg.id_list._length = id_num;
    g_msg.id_list._buffer = id_list;
    g_msg.msg = value_list;
    g_msg.type = DataInfo_SET_SAME_VALUE;
    srand(time(NULL));
    g_msg.random_seq = rand();
    g_msg.binary._buffer = NULL;
    g_msg.binary._maximum = g_msg.binary._length = 0;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (g_reply_num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("set info timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, g_reply_num);
            break;
        }
        dds_transport_try_recv(PAIR_DATA_REPLY, transport, recv_reply);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }
    *info_num = g_reply_num;

    return g_reply_list;
}

int datainfo_server_send_file(struct dds_transport *transport, unsigned long *id_list, int id_num, char *filename, void *pFile, uint32_t file_len)
{
    // clean the reply queue
    g_reply_list = (data_info *) malloc(sizeof(data_info) * id_num);
    g_reply_num = 0;

    // Build up request message
    g_msg.id_list._maximum = g_msg.id_list._length = id_num;
    g_msg.id_list._buffer = id_list;
    g_msg.msg = filename;
    g_msg.type = DataInfo_IMPORT;
    srand(time(NULL));
    g_msg.random_seq = rand();
    g_msg.binary._buffer = pFile;
    g_msg.binary._maximum = g_msg.binary._length = file_len;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);

    // Mark all devices as running
    transfer_result default_result;
    default_result.result = 0;
    default_result.pFile = NULL;
    default_result.file_len = 0;
    for (int i = 0; i < id_num; i++) {
        devinfo_server_set_status_by_id(id_list[i], TRANSFER_RUNNING, default_result);
    }

    // RMT_TODO: I should receive the data in another thread.
    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (g_reply_num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("send file timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, g_reply_num);
            // RMT_TODO: need to check whether the transfer is timeout or not, and then update the status.
            break;
        }
        dds_transport_try_recv(PAIR_DATA_REPLY, transport, recv_file_transfer_reply);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }

    return 0;
}

int datainfo_server_recv_file(struct dds_transport *transport, unsigned long id, char *filename)
{
    // clean the reply queue
    g_reply_list = (data_info *) malloc(sizeof(data_info) * 1);
    g_reply_num = 0;

    // Build up request message
    g_msg.id_list._maximum = g_msg.id_list._length = 1;
    g_msg.id_list._buffer = &id;
    g_msg.msg = filename;
    g_msg.type = DataInfo_EXPORT;
    srand(time(NULL));
    g_msg.random_seq = rand();
    g_msg.binary._buffer = NULL;
    g_msg.binary._maximum = g_msg.binary._length = 0;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);

    // Mark all devices as running
    transfer_result default_result;
    default_result.result = 0;
    default_result.pFile = NULL;
    default_result.file_len = 0;
    devinfo_server_set_status_by_id(id, TRANSFER_RUNNING, default_result);

    // RMT_TODO: I should receive the data in another thread.
    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (g_reply_num != 1) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("recv file timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, 1, g_reply_num);
            break;
        }
        dds_transport_try_recv(PAIR_DATA_REPLY, transport, recv_file_transfer_reply);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }

    return 0;
}