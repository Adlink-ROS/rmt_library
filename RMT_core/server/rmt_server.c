#include "rmt_server.h"
#include "config.h"
#include "device_info.h"

int rmt_server_init(void)
{
    return device_info_init();
}

device_info* rmt_server_get_device_list(uint32_t *num)
{
    device_info *dev;
    device_info_create_list(&dev, num);
    return dev;
}

int rmt_server_free_device_list(device_info **dev)
{
    return device_info_free_list(dev);
}

int rmt_server_get_info(char *key, char *value)
{
    return 0;
}

int rmt_server_set_info(char *key, char *value)
{
    return 0;
}

int rmt_server_send_file(char *filename, void *pFile, uint32_t file_len)
{
    return 0;
}

int rmt_server_recv_file(char *filename, void *pFile, uint32_t *file_len)
{
    return 0;
}

int rmt_server_deinit(void)
{
    return device_info_deinit();
}

char* rmt_server_version(void)
{
    return PROJECT_VERSION;
}