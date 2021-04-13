#include <stdlib.h>
#include <string.h>
#include "dds_transport.h"
#include "rmt_agent.h"
#include "devinfo_agent.h"
#include "datainfo_agent.h"
#include "DataInfo.h"
#include "logger.h"

#define QUEUE_SIZE 16

static datainfo_func *g_datainfo_func_maps;

static DataInfo_Reply datainfo_replys[QUEUE_SIZE];
static int q_front = 0;
static int q_rear = 0;

static int q_enqueue(void)
{
    // queue is full
    if ((q_rear + 1) % QUEUE_SIZE == q_front) {
        return -1;
    }
    q_rear = (q_rear + 1) % QUEUE_SIZE;
    return q_rear;
}

static int q_dequeue(void)
{
    // queue is empty
    if (q_front == q_rear) {
        return -1;
    }
    q_front = (q_front + 1) % QUEUE_SIZE;
    return q_front;
}

static int recv_request(void *msg)
{
    unsigned long myid = devinfo_get_id();
    DataInfo_Request *datainfo_msg = (DataInfo_Request *) msg;

    RMT_LOG("key_list: %s\n", datainfo_msg->msg);
    RMT_LOG("type: %d\n", datainfo_msg->type);
    RMT_LOG("id_list.length: %d\n", datainfo_msg->id_list._length);

    // check whether the ID matches or not
    int found_idx = -1;
    for (int i = 0; i < datainfo_msg->id_list._length; i++) {
        RMT_LOG("id_list.id %d:%lu\n", i, datainfo_msg->id_list._buffer[i]);
        if (myid == datainfo_msg->id_list._buffer[i]) {
            RMT_LOG("The ID matches!\n");
            found_idx = i;
            break;
        }
    }
    // The request is not for me.
    if (found_idx == -1) {
        return 1;
    }

    // If the request is for me, start to get reply from queue.
    int q_idx = q_enqueue();
    datainfo_replys[q_idx].msg = malloc(1024);
    datainfo_replys[q_idx].msg[0] = 0;

    if (datainfo_msg->type == DataInfo_GET) {
        // return the get info back
        datainfo_replys[q_idx].type = DataInfo_GET;
        datainfo_replys[q_idx].deviceID = myid;
        // parse keylist and the return with value
        char *keys = strtok(datainfo_msg->msg, ";");
        while (keys != NULL) {
            RMT_LOG("The key is %s\n", keys);
            for (int i = 0; g_datainfo_func_maps != NULL && g_datainfo_func_maps[i].key != 0; i++) {
                if (strcmp(keys, g_datainfo_func_maps[i].key) == 0) {
                    char value[256];
                    RMT_LOG("match the key!!\n");
                    if (g_datainfo_func_maps[i].get_func) {
                        g_datainfo_func_maps[i].get_func(value);
                    } else {
                        RMT_ERROR("There is no get function for key %s\n", keys);
                    }
                    strcat(datainfo_replys[q_idx].msg, keys);
                    strcat(datainfo_replys[q_idx].msg, ":");
                    strcat(datainfo_replys[q_idx].msg, value);
                    strcat(datainfo_replys[q_idx].msg, ";");
                    break;
                }
            }
            keys = strtok(NULL, ";");
        }
        RMT_LOG("reply message: %s\n", datainfo_replys[q_idx].msg);
    } else if (datainfo_msg->type == DataInfo_SET) {
        // The set message format will be "key1:value1,key2:value2,;key1:value1,key2:value2,;".
        //                                 |----------------------| |----------------------|
        //                                            |                        |
        //                                           id1                      id2
        // That is, the first delimitor is ";", then ",", and finally ":";
        RMT_LOG("recv meg: %s\n", datainfo_msg->msg);
        char *set_pairs_list = strtok(datainfo_msg->msg, ";");
        while (found_idx != 0) {
            set_pairs_list = strtok(NULL, ";");
            found_idx--;
        }
        RMT_LOG("set info: %s\n", set_pairs_list);
        char *set_pairs_list_dup;
        char *pairs = strtok_r(set_pairs_list, ",", &set_pairs_list_dup);
        while (pairs != NULL) {
            RMT_LOG("The pair is %s\n", pairs);
            char *key = strtok(pairs, ":");
            char *value = strtok(NULL, ":");
            for (int i = 0; g_datainfo_func_maps != NULL && g_datainfo_func_maps[i].key != 0; i++) {
                if (strcmp(key, g_datainfo_func_maps[i].key) == 0) {
                    RMT_LOG("match the key!!\n");
                    if (g_datainfo_func_maps[i].set_func) {
                        g_datainfo_func_maps[i].set_func(value);
                    } else {
                        RMT_ERROR("There is no set function for key %s\n", key);
                    }
                    break;
                }
            }
            pairs = strtok_r(NULL, ",", &set_pairs_list_dup);
        }
        // return the set result back
        datainfo_replys[q_idx].type = DataInfo_SET;
        datainfo_replys[q_idx].deviceID = myid;
    } else {
        // wrong type
        RMT_ERROR("Wrong type %d for request.\n", datainfo_msg->type);
        return 1;
    }

    return 0;
}

static int datainfo_agent_send_data(struct dds_transport *transport)
{
    int q_idx;

    while ((q_idx = q_dequeue()) != -1) {
        // reply the data
        if (datainfo_replys[q_idx].msg) {
            dds_transport_send(PAIR_DATA_REPLY, transport, &datainfo_replys[q_idx]);
            free(datainfo_replys[q_idx].msg);
            datainfo_replys[q_idx].msg = NULL;
        }
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
    for (int i = 0; i < QUEUE_SIZE; i++) {
        datainfo_replys[i].msg = NULL;
    }
    return 0;
}
