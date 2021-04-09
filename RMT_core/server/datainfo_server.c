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

    g_reply_list[g_reply_num].deviceID = datainfo_msg->deviceID;
    g_reply_list[g_reply_num].value_list = strdup(datainfo_msg->msg);
    g_reply_num++;
    return 0;
}

data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, int id_num, char *key_list, int *info_num)
{
    g_msg.id_list._maximum = g_msg.id_list._length = id_num;
    g_msg.id_list._buffer = id_list;
    g_msg.msg = key_list;
    g_msg.type = DataInfo_GET;

    // clean the reply queue
    g_reply_list = (data_info *) malloc(sizeof(data_info) * id_num);
    g_reply_num = 0;

    // send request
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);

    time_t start_time, now_time;
    time(&start_time);
    now_time = start_time;
    // wait for all the reply
    while (g_reply_num != id_num && now_time - start_time < DEFAULT_TIMEOUT) {
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
