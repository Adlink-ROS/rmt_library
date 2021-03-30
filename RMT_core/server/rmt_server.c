#include "rmt_server.h"
#include "version.h"
#include "dds_transport.h"
#include "devinfo_server.h"
#include "datainfo_server.h"

int rmt_server_config(char *interface)
{
    return dds_transport_domain_init(interface);
}

int rmt_server_init(void)
{
    return dds_transport_server_init();
}

device_info* rmt_server_create_device_list(int *num)
{
    device_info *dev;
    devinfo_server_create_list(&dev, num);
    return dev;
}

int rmt_server_free_device_list(device_info **dev)
{
    return devinfo_server_free_list(dev);
}

data_info* rmt_server_get_info(unsigned long *id_list, char *key_list, int dev_num)
{
    return datainfo_server_get_info(id_list, key_list, dev_num);
}

int* rmt_server_set_info(unsigned long *id_list, char *key_list, data_info *dev_list, int dev_num)
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
    int ret = dds_transport_deinit();
    devinfo_server_deinit();
    return ret;
}

char* rmt_server_version(void)
{
    return PROJECT_VERSION;
}