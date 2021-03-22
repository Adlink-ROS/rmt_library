#ifndef _RMT_SERVER_
#define _RMT_SERVER_

#include <stdint.h>

typedef struct _device_info {
    int  devcieID;
    char *model;
    char *host;
    char *ip;
    char *mac;
    char *rmt_version;
} device_info;

int rmt_server_init(void);
int rmt_server_list_device(device_info *dev, uint32_t *num);
int rmt_server_get_info(char *key, char *value);
int rmt_server_set_info(char *key, char *value);
int rmt_server_send_file(char *filename, void *pFile, uint32_t file_len);
int rmt_server_recv_file(char *filename, void *pFile, uint32_t *file_len);
int rmt_server_deinit(void);
char* rmt_server_version(void);

#endif /*_RMT_SERVER_*/