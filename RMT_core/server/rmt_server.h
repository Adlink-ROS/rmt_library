#ifndef _RMT_SERVER_
#define _RMT_SERVER_

#include <stdint.h>

// TODO: use char[] instead of char* in the future, or it'll cause memory leak.
typedef struct _device_info {
    uint64_t deviceID;
    char *model;
    char *host;
    char *ip;
    char *mac;
    char *rmt_version;
} device_info;

int rmt_server_init(void);
device_info *rmt_server_create_device_list(uint32_t *num);
int rmt_server_free_device_list(device_info **dev);
int rmt_server_get_info(char *key, char *value);
int rmt_server_set_info(char *key, char *value);
int rmt_server_send_file(char *filename, void *pFile, uint32_t file_len);
int rmt_server_recv_file(char *filename, void *pFile, uint32_t *file_len);
int rmt_server_deinit(void);
char* rmt_server_version(void);

#endif /*_RMT_SERVER_*/