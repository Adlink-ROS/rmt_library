#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "rmt_server.h"
#include "version.h"
#include "dds_transport.h"
#include "devinfo_server.h"
#include "datainfo_server.h"
#include "server_config.h"
#include "logger.h"
#include "network.h"

typedef struct {
    struct dds_transport *transport;
    /* related to recv thread */
    pthread_t recv_thread;
    int recv_thread_status; // 0: stop, 1: running
} rmt_server_info;
static rmt_server_info g_svr_info;

void *recv_thread_func(void *data)
{
    data = data;
    RMT_LOG("Start recv thread.\n")
    while (1 == g_svr_info.recv_thread_status) {
        // If interface changed, we should reinit the server
        if (g_server_cfg.auto_detect_interface) {
            char interface[40];
            net_select_interface(interface);
            if (strcmp(interface, g_server_cfg.net_interface) != 0) {
                RMT_LOG("Interface %s disappear! Reinit communication...\n", g_server_cfg.net_interface);
                if (g_svr_info.transport) {
                    dds_transport_deinit(g_svr_info.transport);
                    g_svr_info.transport = NULL;
                    // clear the remaining device
                    devinfo_server_deinit();
                }
                if (dds_transport_config_init(interface, g_server_cfg.domain_id) < 0) {
                    RMT_ERROR("Unable to init communication\n");
                    continue;
                }
                g_svr_info.transport = dds_transport_server_init(devinfo_server_del_device_callback);
                if (g_svr_info.transport == NULL) {
                    RMT_ERROR("Unable to init agent\n");
                    continue;
                }
                devinfo_server_init();
                datainfo_server_init();
                RMT_LOG("Init interface %s successfully!\n", interface);
                strcpy(g_server_cfg.net_interface, interface);
            }
        }
        devinfo_server_update(g_svr_info.transport);
        dataserver_info_file_transfer_thread(g_svr_info.transport);
        usleep(10000); // sleep 10ms
    }
    RMT_LOG("Stop recv thread.\n")
    pthread_exit(NULL); // leave the thread
}

int rmt_server_configure(char *interface, int domain_id)
{
    return server_config_set(interface, domain_id);
}

int rmt_server_init(void)
{
    dds_transport_config_init(g_server_cfg.net_interface, g_server_cfg.domain_id);
    devinfo_server_init();
    datainfo_server_init();
    g_svr_info.transport = dds_transport_server_init(devinfo_server_del_device_callback);
    if (g_svr_info.transport) {
        RMT_LOG("Init server successfully\n");
        g_svr_info.recv_thread_status = 1;
        pthread_create(&g_svr_info.recv_thread, NULL, recv_thread_func, NULL);
        return 0;
    } else {
        RMT_ERROR("Unable to init server\n");
        return -1;
    }
}

device_info* rmt_server_create_device_list(int *num)
{
    device_info *dev;

    RMT_LOG("Create device list.\n");
    devinfo_server_create_list(g_svr_info.transport, &dev, (unsigned int *)num);
    return dev;
}

int rmt_server_free_device_list(device_info *dev)
{
    RMT_LOG("Free device list.\n");
    return devinfo_server_free_list(dev);
}

data_info* rmt_server_get_info(unsigned long *id_list, int id_num, char *key_list, int *info_num)
{
    RMT_LOG("Get info.\n");
    return datainfo_server_get_info(g_svr_info.transport, id_list, id_num, key_list, info_num);
}

int rmt_server_free_info(data_info* info_list)
{
    RMT_LOG("Free info.\n");
    return datainfo_server_free_info(info_list);
}

data_info* rmt_server_set_info(data_info *dev_list, int dev_num, int *info_num)
{
    RMT_LOG("Set info.\n");
    return datainfo_server_set_info(g_svr_info.transport, dev_list, dev_num, info_num);
}

data_info* rmt_server_set_info_with_same_value(unsigned long *id_list, int id_num, char *value_list, int *info_num)
{
    RMT_LOG("Set info with same value.\n");
    return datainfo_server_set_info_with_same_value(g_svr_info.transport, id_list, id_num, value_list, info_num);
}

int rmt_server_send_file(unsigned long *id_list, int id_num, char *callbackname, char *filename, void *pFile, unsigned long file_len)
{
    RMT_LOG("Send file.\n");
    return datainfo_server_send_file(g_svr_info.transport, id_list, id_num, callbackname, filename, pFile, file_len);
}

int rmt_server_recv_file(unsigned long id, char *callbackname, char *filename)
{
    RMT_LOG("Receive file.\n");
    return datainfo_server_recv_file(g_svr_info.transport, id, callbackname, filename);
}

transfer_status rmt_server_get_result(unsigned long device_id, transfer_result *result)
{
    transfer_status current_status;

    RMT_LOG("Get result.\n");
    devinfo_server_get_status_by_id(device_id, &current_status, result);
    return current_status;
}

int rmt_server_deinit(void)
{
    int ret;

    RMT_LOG("Deinit RMT server.\n")
    // kill the thread
    g_svr_info.recv_thread_status = 0;
    pthread_join(g_svr_info.recv_thread, NULL);

    ret = dds_transport_deinit(g_svr_info.transport);
    devinfo_server_deinit();
    return ret;
}

char* rmt_server_version(void)
{
    RMT_LOG("Get RMT version.\n");
    return PROJECT_VERSION;
}
