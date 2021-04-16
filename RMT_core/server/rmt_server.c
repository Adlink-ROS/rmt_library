#include "rmt_server.h"
#include "version.h"
#include "dds_transport.h"
#include "devinfo_server.h"
#include "datainfo_server.h"
#include "logger.h"

static struct dds_transport *g_transport;

int rmt_server_config(char *interface)
{
    return dds_transport_config_init(interface);
}

int rmt_server_init(void)
{
    g_transport = dds_transport_server_init();
    if (g_transport) {
        return 0;
    } else {
        RMT_ERROR("Unable to init server\n");
        return -1;
    }
}

device_info* rmt_server_create_device_list(int *num)
{
    device_info *dev;

    devinfo_server_create_list(g_transport, &dev, num);
    return dev;
}

int rmt_server_free_device_list(device_info *dev)
{
    return devinfo_server_free_list(dev);
}

data_info* rmt_server_get_info(unsigned long *id_list, int id_num, char *key_list, int *info_num)
{
    return datainfo_server_get_info(g_transport, id_list, id_num, key_list, info_num);
}

int rmt_server_free_info(data_info* info_list, int info_num)
{
    return datainfo_server_free_info(info_list, info_num);
}

data_info* rmt_server_set_info(data_info *dev_list, int dev_num, int *info_num)
{
    return datainfo_server_set_info(g_transport, dev_list, dev_num, info_num);
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
    int ret = dds_transport_deinit(g_transport);

    devinfo_server_deinit();
    return ret;
}

char* rmt_server_version(void)
{
    return PROJECT_VERSION;
}
