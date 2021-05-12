#include <pthread.h>
#include "rmt_server.h"
#include "version.h"
#include "dds_transport.h"
#include "devinfo_server.h"
#include "datainfo_server.h"
#include "logger.h"

static struct dds_transport *g_transport;
static pthread_t g_recv_thread;
int g_recv_thread_status; // 1: running, 0 stop

void *recv_thread_func(void *data) {
    while (1 == g_recv_thread_status) {
    }
    pthread_exit(NULL); // leave the thread
}

int rmt_server_config(char *interface)
{
    return dds_transport_config_init(interface);
}

int rmt_server_init(void)
{
    g_transport = dds_transport_server_init(devinfo_server_del_device_callback);
    if (g_transport) {
        g_recv_thread_status = 1;
        pthread_create(&g_recv_thread, NULL, recv_thread_func, NULL);
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

int rmt_server_free_info(data_info* info_list)
{
    return datainfo_server_free_info(info_list);
}

data_info* rmt_server_set_info(data_info *dev_list, int dev_num, int *info_num)
{
    return datainfo_server_set_info(g_transport, dev_list, dev_num, info_num);
}

data_info* rmt_server_set_info_with_same_value(unsigned long *id_list, int id_num, char *value_list, int *info_num)
{
    return datainfo_server_set_info_with_same_value(g_transport, id_list, id_num, value_list, info_num);
}

int rmt_server_send_file(unsigned long *id_list, int id_num, char *filename, void *pFile, uint32_t file_len)
{
    // RMT_TODO: Suggest to run server update in another thread.
    devinfo_server_update(g_transport);
    return datainfo_server_send_file(g_transport, id_list, id_num, filename, pFile, file_len);
}

int rmt_server_recv_file(unsigned long id, char *filename)
{
    // RMT_TODO: Suggest to run server update in another thread.
    devinfo_server_update(g_transport);
    return datainfo_server_recv_file(g_transport, id, filename);
}

transfer_status rmt_server_get_result(unsigned long device_id, transfer_result *result)
{
    transfer_status current_status;

    devinfo_server_get_status_by_id(device_id, &current_status, result);
    return current_status;
}

int rmt_server_deinit(void)
{
    int ret = dds_transport_deinit(g_transport);

    devinfo_server_deinit();
    // kill the thread
    g_recv_thread_status = 0;
    pthread_join(g_recv_thread, NULL);
    return ret;
}

char* rmt_server_version(void)
{
    return PROJECT_VERSION;
}
