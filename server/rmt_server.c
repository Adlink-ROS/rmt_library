#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "rmt_server.h"
#include "version.h"
#include "dds_transport.h"
#include "devinfo_server.h"
#include "datainfo_server.h"
#include "network.h"
#include "logger.h"

static struct dds_transport *g_transport;
static pthread_t g_recv_thread;
int g_recv_thread_status = 0; // 0: stop, 1: running

void *recv_thread_func(void *data)
{
    RMT_LOG("Start recv thread.\n")
    while (1 == g_recv_thread_status) {
        devinfo_server_update(g_transport);
        dataserver_info_file_transfer_thread(g_transport);
        usleep(10000); // sleep 10ms
    }
    RMT_LOG("Stop recv thread.\n")
    pthread_exit(NULL); // leave the thread
}

int rmt_server_configure(char *interface)
{
    char myinterface[40];

    if (interface == NULL) {
        if (net_select_interface(myinterface) < 0) {
            RMT_ERROR("Unable to select interface.\n");
            return -1;
        }
    } else {
        strncpy(myinterface, interface, sizeof(myinterface));
    }
    return dds_transport_config_init(myinterface);
}

int rmt_server_init(void)
{
    devinfo_server_init();
    datainfo_server_init();
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

int rmt_server_send_file(unsigned long *id_list, int id_num, char *callbackname, char *filename, void *pFile, unsigned long file_len)
{
    return datainfo_server_send_file(g_transport, id_list, id_num, callbackname, filename, pFile, file_len);
}

int rmt_server_recv_file(unsigned long id, char *callbackname, char *filename)
{
    return datainfo_server_recv_file(g_transport, id, callbackname, filename);
}

transfer_status rmt_server_get_result(unsigned long device_id, transfer_result *result)
{
    transfer_status current_status;

    devinfo_server_get_status_by_id(device_id, &current_status, result);
    return current_status;
}

int rmt_server_deinit(void)
{
    int ret;

    // kill the thread
    g_recv_thread_status = 0;
    pthread_join(g_recv_thread, NULL);

    ret = dds_transport_deinit(g_transport);
    devinfo_server_deinit();
    return ret;
}

char* rmt_server_version(void)
{
    return PROJECT_VERSION;
}
