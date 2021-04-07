#ifndef _RMT_SERVER_
#define _RMT_SERVER_

#include <stdint.h>

// RMT_TODO: use char[] instead of char* in the future, or it'll cause memory leak.
typedef struct _device_info {
    unsigned long deviceID;
    char *model;
    char *host;
    char *ip;
    char *mac;
    char *rmt_version;
} device_info;

typedef struct _data_info {
    unsigned long deviceID;
    char *value_list;
} data_info;

int rmt_server_config(char *interface);
int rmt_server_init(void);
device_info *rmt_server_create_device_list(int *num);
int rmt_server_free_device_list(device_info *dev);
/*
 * argument:
 *   id_list: unsigned long array
 *   id_num: the number of id
 *   key_list: the string of key and each key is splitted with ";", for example "cpu;ram;..."
 *   info_num: the number of return data_info
 * return value:
 *   data_info: data_info array. value_list is the string of key-value pairs, for example "cpu:20;ram:30;..."
 */
data_info* rmt_server_get_info(unsigned long *id_list, int id_num, char *key_list, int *info_num);
int rmt_server_free_info(data_info* info_list, int info_num);
/*
 * argument:
 *   data_info: data_info array. value_list is the string of key-value pairs, for example "cpu:20;ram:30;..."
 *   dev_num: the number of device
 * return value:
 *   return the result of each settings.
 */
int* rmt_server_set_info(data_info *dev_list, int dev_num);
int rmt_server_send_file(char *filename, void *pFile, uint32_t file_len);
int rmt_server_recv_file(char *filename, void *pFile, uint32_t *file_len);
int rmt_server_deinit(void);
char* rmt_server_version(void);

#endif /*_RMT_SERVER_*/