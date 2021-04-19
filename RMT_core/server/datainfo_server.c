#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "dds_transport.h"
#include "rmt_server.h"
#include "DataInfo.h"
#include "logger.h"

#define DEFAULT_TIMEOUT 3

static DataInfo_Request g_msg;
static data_info *g_reply_list;
static int g_reply_num;

static int recv_reply(void *msg)
{
    DataInfo_Reply *datainfo_msg = (DataInfo_Reply *) msg;

    // Check whether this reply is for me or not.
    if ((datainfo_msg->type != g_msg.type) || (datainfo_msg->random_seq != g_msg.random_seq)) {
        return -1;
    }

    RMT_LOG("Receive device ID: %ld\n", datainfo_msg->deviceID);
    g_reply_list[g_reply_num].deviceID = datainfo_msg->deviceID;
    g_reply_list[g_reply_num].value_list = strdup(datainfo_msg->msg);
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

int datainfo_server_free_info(data_info* info_list, int info_num)
{
    for (int i = 0; i < info_num; i++) {
        if (info_list[i].value_list) {
            free(info_list[i].value_list);
            info_list[i].value_list = NULL;
        }
    }
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

    free(id_list);
    free(buffer);

    return g_reply_list;
}
