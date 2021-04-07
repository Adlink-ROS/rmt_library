#include <stdlib.h>
#include <string.h>
#include "dds_transport.h"
#include "rmt_agent.h"
#include "devinfo_agent.h"
#include "datainfo_agent.h"
#include "DataInfo.h"
#include "logger.h"

static datainfo_func *g_datainfo_func_maps;

// RMT_TODO: This should be a queue
DataInfo_Reply datainfo_reply;

static int recv_request(void *msg)
{
    unsigned long myid = devinfo_get_id();
    DataInfo_Request *datainfo_msg = (DataInfo_Request *) msg;
    datainfo_reply.msg = malloc(1024);

    RMT_LOG("key_list: %s\n", datainfo_msg->msg);
    RMT_LOG("type: %d\n", datainfo_msg->type);
    RMT_LOG("id_list.length: %d\n", datainfo_msg->id_list._length);
    // check whether the ID matches or not
    int dev_found = 0;
    for (int i = 0; i < datainfo_msg->id_list._length; i++) {
        RMT_LOG("id_list.id %d:%lu\n", i, datainfo_msg->id_list._buffer[i]);
        if (myid == datainfo_msg->id_list._buffer[i]) {
            RMT_LOG("The ID matches!\n");
            dev_found = 1;
            break;
        }
    }
    // The request is not for me.
    if (!dev_found) return 1;

    if (datainfo_msg->type == DataInfo_GET) {
        // return the get info back
        datainfo_reply.type = DataInfo_GET;
        datainfo_reply.deviceID = myid;
        // parse keylist and the return with value
        // RMT_TODO: need to fill out the corect reply message
        // RMT_TODO: we need to able to pass the handling function into here
        char *keys = strtok(datainfo_msg->msg, ";");
        while (keys != NULL) {
            RMT_LOG("The key is %s\n", keys)
            for (int i = 0; g_datainfo_func_maps != NULL && g_datainfo_func_maps[i].key != 0; i++) {
                if (strcmp(keys, g_datainfo_func_maps[i].key) == 0) {
                    char value[256];
                    RMT_LOG("match the key!!\n");
                    g_datainfo_func_maps[i].get_func(value);
                    strcat(datainfo_reply.msg, keys);
                    strcat(datainfo_reply.msg, ":");
                    strcat(datainfo_reply.msg, value);
                    strcat(datainfo_reply.msg, ";");
                    break;
                }
            }
            keys = strtok(NULL, ";");
        }
        RMT_LOG("reply message: %s\n", datainfo_reply.msg);
    } else if (datainfo_msg->type == DataInfo_SET) {
        // set the info
    } else {
        // wrong type
        return 1;
    }

    return 0;
}

static int datainfo_agent_send_data(struct dds_transport *transport)
{
    // reply the data
    if (datainfo_reply.msg) {
        dds_transport_send(PAIR_DATA_REPLY, transport, &datainfo_reply);
        free(datainfo_reply.msg);
        datainfo_reply.msg = NULL;
    }
    return 0;
}

int datainfo_agent_update(struct dds_transport *transport)
{
    dds_transport_try_recv(PAIR_DATA_REQ, transport, recv_request);
    datainfo_agent_send_data(transport);
    return 0;
}

int datainfo_agent_init(datainfo_func *func_maps)
{
    g_datainfo_func_maps = func_maps;
    datainfo_reply.msg = NULL;
    return 0;
}