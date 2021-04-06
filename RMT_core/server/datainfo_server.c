#include <stdlib.h>
#include <unistd.h>
#include "dds_transport.h"
#include "rmt_server.h"
#include "DataInfo.h"

static DataInfo_Request g_msg;

static int recv_reply(void *msg)
{
    // RMT_TODO: parse the correct format
    DataInfo_Reply *datainfo_msg = (DataInfo_Reply *) msg;
    printf("type: %d\n", datainfo_msg->type);
    printf("id: %d\n", datainfo_msg->deviceID);
    printf("msg: %s\n", datainfo_msg->msg);
    // RMT_TODO: return the data back to user
    return 0;
}

data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, char *key_list, int dev_num)
{
    g_msg.id_list._maximum = g_msg.id_list._length = dev_num;
    g_msg.id_list._buffer = id_list; 
    g_msg.msg = key_list;
    g_msg.type = DataInfo_GET;
    dds_transport_send(PAIR_DATA_REQ, transport, &g_msg);
    // RMT_TODO: setup timeout
    sleep(3);
    dds_transport_try_recv(PAIR_DATA_REPLY, transport, recv_reply);
    return NULL;
}