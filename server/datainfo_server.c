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

typedef struct _reply_data {
    struct DataInfo_Request *req;
    data_info *list;
    int num;
} reply_data;

typedef struct _file_transfer_stat {
    int status; // 0: idle, 1: running
    time_t start_time;
    DataInfo_TYPE type;
    uint64_t random_seq;
    unsigned long *id_list;
    int id_num;
    int recv_num;
} file_transfer_stat;

file_transfer_stat g_file_transfer_stat;

static int recv_reply(void *msg, void *recv_buf, void *arg)
{
    reply_data *replys = (reply_data *)recv_buf;
    DataInfo_Reply *datainfo_msg = (DataInfo_Reply *) msg;

    // Check whether this reply is for me or not.
    if ((datainfo_msg->type != replys->req->type) || (datainfo_msg->random_seq != replys->req->random_seq)) {
        return -1;
    }

    RMT_LOG("Receive device ID: %ld\n", datainfo_msg->deviceID);
    replys->list[replys->num].deviceID = datainfo_msg->deviceID;
    strncpy(replys->list[replys->num].value_list, datainfo_msg->msg, CONFIG_KEY_STR_LEN);
    replys->num++;
    return 0;
}

static int recv_file_transfer_reply(void *msg, void *recv_buf, void *arg)
{
    DataInfo_Reply *datainfo_msg = (DataInfo_Reply *) msg;
    transfer_status status;

    // Check whether this reply is for me or not.
    if ((datainfo_msg->type != g_file_transfer_stat.type) || (datainfo_msg->random_seq != g_file_transfer_stat.random_seq)) {
        return -1;
    }

    RMT_LOG("Receive device ID: %ld\n", datainfo_msg->deviceID);
    RMT_LOG("Receive result: %s\n", datainfo_msg->msg);
    transfer_result file_result;
    file_result.result = atoi(datainfo_msg->msg);
    if (datainfo_msg->type == DataInfo_IMPORT) {
        file_result.pFile = NULL;
        file_result.file_len = 0;
        status = STATUS_DONE;
    } else if (datainfo_msg->type == DataInfo_EXPORT) {
        file_result.pFile = malloc(sizeof(uint8_t) * datainfo_msg->binary._length);
        if (file_result.pFile) {
            memcpy(file_result.pFile, datainfo_msg->binary._buffer, datainfo_msg->binary._length);
            file_result.file_len = datainfo_msg->binary._length;
            status = STATUS_DONE;
        } else {
            status = STATUS_SERVER_ERROR;
        }
    }
    devinfo_server_set_status_by_id(datainfo_msg->deviceID, status, file_result);
    g_file_transfer_stat.recv_num++;
    return 0;
}

void datainfo_server_init(void)
{
    g_file_transfer_stat.status = 0;
    g_file_transfer_stat.start_time = 0;
    g_file_transfer_stat.recv_num = 0;
    g_file_transfer_stat.id_list = NULL;
    g_file_transfer_stat.id_num = 0;
}

data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, int id_num, char *key_list, int *info_num)
{
    static DataInfo_Request req_msg;
    reply_data replys;
    DataInfo_Reply reply_instance;

    // clean the reply queue
    replys.req = &req_msg;
    replys.list = (data_info *) malloc(sizeof(data_info) * id_num);
    replys.num = 0;

    // Build up request message
    req_msg.id_list._maximum = req_msg.id_list._length = id_num;
    req_msg.id_list._buffer = id_list;
    req_msg.msg = key_list;
    req_msg.type = DataInfo_GET;
    srand(time(NULL));
    req_msg.random_seq = rand();
    req_msg.binary._buffer = NULL;
    req_msg.binary._maximum = req_msg.binary._length = 0;

    // Setup instance
    // RMT_TODO: we should make sure random sequence will not duplicate
    // RMT_TODO: we should make sure unlimited key will not cause problem
    reply_instance.random_seq = req_msg.random_seq;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &req_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (replys.num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("get info timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, replys.num);
            break;
        }
        dds_transport_try_recv_instance(&reply_instance, PAIR_DATA_REPLY, transport, recv_reply, &replys);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }
    *info_num = replys.num;

    return replys.list;
}

int datainfo_server_free_info(data_info* info_list)
{
    free(info_list);
    return 0;
}

data_info* datainfo_server_set_info(struct dds_transport *transport, data_info *dev_list, int dev_num, int *info_num)
{
    static DataInfo_Request req_msg;
    reply_data replys;
    DataInfo_Reply reply_instance;

    // clean the reply queue
    replys.req = &req_msg;
    replys.list = (data_info *) malloc(sizeof(data_info) * dev_num);
    replys.num = 0;

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
    req_msg.id_list._maximum = req_msg.id_list._length = id_num;
    req_msg.id_list._buffer = id_list;
    req_msg.msg = buffer;
    req_msg.type = DataInfo_SET;
    srand(time(NULL));
    req_msg.random_seq = rand();
    req_msg.binary._buffer = NULL;
    req_msg.binary._maximum = req_msg.binary._length = 0;

    // Setup instance
    reply_instance.random_seq = req_msg.random_seq;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &req_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (replys.num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("set info timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, replys.num);
            break;
        }
        dds_transport_try_recv_instance(&reply_instance, PAIR_DATA_REPLY, transport, recv_reply, &replys);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }
    *info_num = replys.num;

    free(id_list);
    free(buffer);

    return replys.list;
}

data_info* datainfo_server_set_info_with_same_value(struct dds_transport *transport, unsigned long *id_list, int id_num, char *value_list, int *info_num)
{
    static DataInfo_Request req_msg;
    reply_data replys;
    DataInfo_Reply reply_instance;

    // clean the reply queue
    replys.req = &req_msg;
    replys.list = (data_info *) malloc(sizeof(data_info) * id_num);
    replys.num = 0;

    // Build up request message
    req_msg.id_list._maximum = req_msg.id_list._length = id_num;
    req_msg.id_list._buffer = id_list;
    req_msg.msg = value_list;
    req_msg.type = DataInfo_SET_SAME_VALUE;
    srand(time(NULL));
    req_msg.random_seq = rand();
    req_msg.binary._buffer = NULL;
    req_msg.binary._maximum = req_msg.binary._length = 0;

    // Setup instance
    reply_instance.random_seq = req_msg.random_seq;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &req_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (replys.num != id_num) {
        if (now_time - start_time > DEFAULT_TIMEOUT) {
            RMT_WARN("set info timeout: %d, expect %d, but receive %d.\n", DEFAULT_TIMEOUT, id_num, replys.num);
            break;
        }
        dds_transport_try_recv_instance(&reply_instance, PAIR_DATA_REPLY, transport, recv_reply, &replys);
        usleep(10000); // sleep 10ms
        time(&now_time);
    }
    *info_num = replys.num;

    return replys.list;
}

int datainfo_server_send_file(struct dds_transport *transport, unsigned long *id_list, int id_num, char *callbackname, char *filename, void *pFile, uint32_t file_len)
{
    static DataInfo_Request req_msg;
    char message[1024] = {0};

    // Last file transfer hasn't finished.
    if (g_file_transfer_stat.status != 0) {
        return -1;
    }
    strcat(message, callbackname);
    strcat(message, ";");
    strcat(message, filename);

    // Build up request message
    req_msg.id_list._maximum = req_msg.id_list._length = id_num;
    req_msg.id_list._buffer = id_list;
    req_msg.msg = message;
    req_msg.type = DataInfo_IMPORT;
    srand(time(NULL));
    req_msg.random_seq = rand();
    req_msg.binary._buffer = pFile;
    req_msg.binary._maximum = req_msg.binary._length = file_len;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &req_msg);

    // Mark all devices as running
    for (int i = 0; i < id_num; i++) {
        devinfo_server_set_status_by_id(id_list[i], STATUS_RUNNING, empty_result);
    }

    // Trigger the file transfer to run
    g_file_transfer_stat.status = 1;
    g_file_transfer_stat.recv_num = 0;
    g_file_transfer_stat.random_seq = req_msg.random_seq;
    g_file_transfer_stat.type = req_msg.type;
    g_file_transfer_stat.id_num = id_num;
    g_file_transfer_stat.id_list = malloc(sizeof(unsigned long) * id_num);
    for (int i = 0; i < id_num; i++) {
        g_file_transfer_stat.id_list[i] = id_list[i];
    }
    time(&g_file_transfer_stat.start_time);

    return 0;
}

int datainfo_server_recv_file(struct dds_transport *transport, unsigned long id, char *callbackname, char *filename)
{
    static DataInfo_Request req_msg;
    char message[1024] = {0};

    // Last file transfer hasn't finished.
    if (g_file_transfer_stat.status != 0) {
        return -1;
    }
    strcat(message, callbackname);
    strcat(message, ";");
    strcat(message, filename);

    // Build up request message
    req_msg.id_list._maximum = req_msg.id_list._length = 1;
    req_msg.id_list._buffer = &id;
    req_msg.msg = message;
    req_msg.type = DataInfo_EXPORT;
    srand(time(NULL));
    req_msg.random_seq = rand();
    req_msg.binary._buffer = NULL;
    req_msg.binary._maximum = req_msg.binary._length = 0;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &req_msg);

    // Mark all devices as running
    devinfo_server_set_status_by_id(id, STATUS_RUNNING, empty_result);

    // Trigger the file transfer to run
    g_file_transfer_stat.status = 1;
    g_file_transfer_stat.recv_num = 0;
    g_file_transfer_stat.random_seq = req_msg.random_seq;
    g_file_transfer_stat.type = req_msg.type;
    g_file_transfer_stat.id_num = 1;
    g_file_transfer_stat.id_list = malloc(sizeof(unsigned long) * 1);
    g_file_transfer_stat.id_list[0] = id;
    time(&g_file_transfer_stat.start_time);

    return 0;
}

void dataserver_info_file_transfer_thread(struct dds_transport *transport)
{
    time_t now_time;
    DataInfo_Reply reply_instance;

    // Setup instance
    reply_instance.random_seq = g_file_transfer_stat.random_seq;

    if (g_file_transfer_stat.status == 1) {
        // Try to receive data from agent
        for (int i = g_file_transfer_stat.recv_num; i < g_file_transfer_stat.id_num; i++) {
            dds_transport_try_recv_instance(&reply_instance, PAIR_DATA_REPLY, transport, recv_file_transfer_reply, NULL);
        }
        // If we receive all the data
        if (g_file_transfer_stat.recv_num == g_file_transfer_stat.id_num) {
            g_file_transfer_stat.status = 0;
        }
        // While receive timeout
        time(&now_time);
        if (now_time - g_file_transfer_stat.start_time > DEFAULT_TIMEOUT) {
            for (int i = 0; i < g_file_transfer_stat.id_num; i++) {
                transfer_status dev_status;
                transfer_result dev_result;
                devinfo_server_get_status_by_id(g_file_transfer_stat.id_list[i], &dev_status, &dev_result);
                // Mark all running device as agent error
                if (dev_status == STATUS_RUNNING) {
                    devinfo_server_set_status_by_id(g_file_transfer_stat.id_list[i], STATUS_AGENT_ERROR, empty_result);
                }
            }
            g_file_transfer_stat.status = 0;
        }
        // free the resource
        if (g_file_transfer_stat.status == 0) {
            g_file_transfer_stat.id_list = NULL;
            g_file_transfer_stat.id_num = 0;
        }
    }
}
