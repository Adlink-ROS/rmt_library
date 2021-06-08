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
static fileinfo_func *g_fileinfo_func_maps;
unsigned long g_datainfo_val_size;

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

static int recv_request(void *msg, void *arg, void *recv_buf)
{
    unsigned long myid = devinfo_get_id();
    DataInfo_Request *datainfo_msg = (DataInfo_Request *) msg;

    arg = arg;
    recv_buf = recv_buf;

    RMT_LOG("id_list.length: %d\n", datainfo_msg->id_list._length);
    RMT_LOG("type: %d\n", datainfo_msg->type);
    RMT_LOG("msg: %s\n", datainfo_msg->msg);

    // check whether the ID matches or not
    int found_idx = -1;
    for (unsigned int i = 0; i < datainfo_msg->id_list._length; i++) {
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

    if (datainfo_msg->type == DataInfo_GET) {
        unsigned int result_msg_size = 1024;
        char *result_msg = calloc(sizeof(char), result_msg_size);
        // parse keylist and the return with value
        char *keys = strtok(datainfo_msg->msg, ";");
        while (keys != NULL) {
            RMT_LOG("The key is %s\n", keys);
            for (int i = 0; g_datainfo_func_maps != NULL && g_datainfo_func_maps[i].key != 0; i++) {
                if (strcmp(keys, g_datainfo_func_maps[i].key) == 0) {
                    char *value = calloc(sizeof(char), g_datainfo_val_size);
                    RMT_LOG("match the key!!\n");
                    if (g_datainfo_func_maps[i].get_func) {
                        g_datainfo_func_maps[i].get_func(value);
                    } else {
                        RMT_ERROR("There is no get function for key %s\n", keys);
                    }
                    // Make sure result_msg length is enough
                    unsigned int new_size = strlen(result_msg) + strlen(keys) + strlen(value) + 2;
                    if (result_msg_size <= new_size) {
                        result_msg_size = new_size * 2;
                        result_msg = realloc(result_msg, result_msg_size);
                    }
                    strcat(result_msg, keys);
                    strcat(result_msg, ":");
                    strcat(result_msg, value);
                    strcat(result_msg, ";");
                    free(value);
                    break;
                }
            }
            keys = strtok(NULL, ";");
        }
        // return the get info back
        datainfo_replys[q_idx].type = DataInfo_GET;
        datainfo_replys[q_idx].random_seq = datainfo_msg->random_seq;
        datainfo_replys[q_idx].deviceID = myid;
        datainfo_replys[q_idx].msg = result_msg;
        datainfo_replys[q_idx].binary._buffer = NULL;
        datainfo_replys[q_idx].binary._maximum = datainfo_replys[q_idx].binary._length = 0;
        RMT_LOG("reply message: %s\n", datainfo_replys[q_idx].msg);
    } else if ((datainfo_msg->type == DataInfo_SET) || (datainfo_msg->type == DataInfo_SET_SAME_VALUE)) {
        char *result_msg = calloc(sizeof(char), 1024);
        RMT_LOG("recv meg: %s\n", datainfo_msg->msg);
        char *set_pairs_list;
        if (datainfo_msg->type == DataInfo_SET) {
            // The set message format will be "key1:value1;key2:value2;|key1:value1;key2:value2;".
            //                                 |----------------------| |----------------------|
            //                                            |                        |
            //                                           id1                      id2
            // That is, the first delimitor is ";", then ",", and finally ":";
            set_pairs_list = strtok(datainfo_msg->msg, "|");
            while (found_idx != 0) {
                set_pairs_list = strtok(NULL, "|");
                found_idx--;
            }
        } else { // DataInfo_SET_SAME_VALUE
            // The format of DataInfo_SET_SAME_VALUE is much more simpler, "key1:valu1;key2:value2;"
            set_pairs_list = datainfo_msg->msg;
        }
        RMT_LOG("set info: %s\n", set_pairs_list);
        char *set_pairs_list_dup;
        char *pairs = strtok_r(set_pairs_list, ";", &set_pairs_list_dup);
        while (pairs != NULL) {
            RMT_LOG("The pair is %s\n", pairs);
            char *key = strtok(pairs, ":");
            // RMT_TODO: There will be problem if there is ":" in user's value. We use workaround here but it should be fixed in the future.
            //char *value = strtok(NULL, ":");
            char *value = pairs + strlen(key) + 1;
            for (int i = 0; g_datainfo_func_maps != NULL && g_datainfo_func_maps[i].key != 0; i++) {
                if (strcmp(key, g_datainfo_func_maps[i].key) == 0) {
                    RMT_LOG("match the key!!\n");
                    if (g_datainfo_func_maps[i].set_func) {
                        char msg[128];
                        int result = g_datainfo_func_maps[i].set_func(value);
                        sprintf(msg, "%s:%d;", key, result);
                        strcat(result_msg, msg);
                    } else {
                        RMT_ERROR("There is no set function for key %s\n", key);
                    }
                    break;
                }
            }
            pairs = strtok_r(NULL, ";", &set_pairs_list_dup);
        }
        // return the set result back
        datainfo_replys[q_idx].type = datainfo_msg->type;
        datainfo_replys[q_idx].random_seq = datainfo_msg->random_seq;
        datainfo_replys[q_idx].deviceID = myid;
        datainfo_replys[q_idx].msg = result_msg;
        datainfo_replys[q_idx].binary._buffer = NULL;
        datainfo_replys[q_idx].binary._maximum = datainfo_replys[q_idx].binary._length = 0;
    } else if (datainfo_msg->type == DataInfo_IMPORT) {
        char *result_msg = calloc(sizeof(char), 1024);
        char *callbackname, *filename;
        callbackname = strtok(datainfo_msg->msg, ";");
        filename = strtok(NULL, ";");
        // run import function
        for (int i = 0; g_fileinfo_func_maps != NULL && g_fileinfo_func_maps[i].callbackname != 0; i++) {
            if (strcmp(callbackname, g_fileinfo_func_maps[i].callbackname) == 0) {
                RMT_LOG("match the callback name!!\n");
                // save the file
                if (g_fileinfo_func_maps[i].path != NULL) {
                    char filepath[1024] = {0};
                    sprintf(filepath, "%s/%s", g_fileinfo_func_maps[i].path, filename);
                    FILE *fp = fopen(filepath, "w");
                    fwrite(datainfo_msg->binary._buffer, 1, datainfo_msg->binary._length, fp);
                    fclose(fp);
                }
                if (g_fileinfo_func_maps[i].import_func) {
                    int result = g_fileinfo_func_maps[i].import_func(filename, (char *) datainfo_msg->binary._buffer, datainfo_msg->binary._length);
                    sprintf(result_msg, "%d", result);
                    RMT_LOG("result of import func: %d\n", result);
                } else {
                    RMT_ERROR("There is no import function for filename %s\n", datainfo_msg->msg);
                }
                break;
            }
        }
        // RMT_TODO: What if there is no match filename? We need to modify the return message like filename=xxxx
        // build reply message
        datainfo_replys[q_idx].type = datainfo_msg->type;
        datainfo_replys[q_idx].random_seq = datainfo_msg->random_seq;
        datainfo_replys[q_idx].deviceID = myid;
        datainfo_replys[q_idx].msg = result_msg;
        datainfo_replys[q_idx].binary._buffer = NULL;
        datainfo_replys[q_idx].binary._maximum = datainfo_replys[q_idx].binary._length = 0;
    } else if (datainfo_msg->type == DataInfo_EXPORT) {
        char *result_msg = calloc(sizeof(char), 1024);
        unsigned char *file_content = NULL;
        unsigned long file_size = 0;
        char *callbackname, *filename;
        callbackname = strtok(datainfo_msg->msg, ";");
        filename = strtok(NULL, ";");
        // run export function
        for (int i = 0; g_fileinfo_func_maps != NULL && g_fileinfo_func_maps[i].callbackname != 0; i++) {
            if (strcmp(callbackname, g_fileinfo_func_maps[i].callbackname) == 0) {
                RMT_LOG("match the callback name!!\n");
                // read the file
                if (g_fileinfo_func_maps[i].path != NULL) {
                    char filepath[1024] = {0};
                    sprintf(filepath, "%s/%s", g_fileinfo_func_maps[i].path, filename);
                    FILE *fp = fopen(filepath, "r");
                    if (fp != NULL) {
                        fseek(fp, 0L, SEEK_END);
                        file_size = ftell(fp);
                        fseek(fp, 0L, SEEK_SET);
                        file_content = malloc(file_size);
                        if (fread(file_content, 1, file_size, fp) != file_size) {
                            RMT_WARN("something wrong while reading file\n");
                        }
                        fclose(fp);
                    }
                }
                if (g_fileinfo_func_maps[i].export_func) {
                    // RMT_TODO: Should we consider export function will extent the file length?
                    int result = g_fileinfo_func_maps[i].export_func(filename, (char *) file_content, file_size);
                    sprintf(result_msg, "%d", result);
                    RMT_LOG("result of export func: %d\n", result);
                } else {
                    RMT_ERROR("There is no export function for filename %s\n", datainfo_msg->msg);
                }
                break;
            }
        }
        // RMT_TODO: What if there is no match filename? We need to modify the return message like filename=xxxx
        // build reply message
        datainfo_replys[q_idx].type = datainfo_msg->type;
        datainfo_replys[q_idx].random_seq = datainfo_msg->random_seq;
        datainfo_replys[q_idx].deviceID = myid;
        datainfo_replys[q_idx].msg = result_msg;
        datainfo_replys[q_idx].binary._buffer = file_content;
        datainfo_replys[q_idx].binary._maximum = datainfo_replys[q_idx].binary._length = file_size;
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
    dds_transport_try_recv(PAIR_DATA_REQ, transport, recv_request, NULL);
    datainfo_agent_send_data(transport);
    return 0;
}

int datainfo_agent_init(datainfo_func *func_maps, fileinfo_func *file_maps, unsigned long datainfo_val_size)
{
    g_datainfo_func_maps = func_maps;
    g_fileinfo_func_maps = file_maps;
    g_datainfo_val_size = datainfo_val_size;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        datainfo_replys[i].msg = NULL;
    }
    return 0;
}
