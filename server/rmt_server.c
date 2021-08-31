#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "rmt_server.h"
#include "version.h"
#include "dds_transport.h"
#include "devinfo_server.h"
#include "datainfo_server.h"
#include "logger.h"
#include "network.h"
#include "rmt_config.h"

typedef enum {
    SVR_STAT_STOP,
    SVR_STAT_RUNNING,
} server_status;

typedef struct {
    struct dds_transport *transport;
    /* related to recv thread */
    pthread_t recv_thread;
    /* whether recv thread is running or not. 0: stop, 1: running*/
    int recv_thread_status;
    /* server current status */
    server_status status;
    /* how many api is used now. */
    unsigned long using_api;
} rmt_server_info;
static rmt_server_info g_svr_info;

void *recv_thread_func(void *data)
{
    data = data;
    RMT_LOG("Start recv thread.\n")
    while (1 == g_svr_info.recv_thread_status) {
        if (g_svr_info.transport) {
            devinfo_server_update(g_svr_info.transport);
            dataserver_info_file_transfer_thread(g_svr_info.transport);
        }
        // If interface changed, we should reinit the server
        if ((g_rmt_cfg.auto_detect_interface) || (g_svr_info.status == SVR_STAT_STOP)) {
            char interface[40] = {0};
            char ip[40] = {0};

            net_select_interface(interface);
            net_get_ip(interface, ip, sizeof(ip));
            if ((strcmp(interface, g_rmt_runtime_cfg.net_interface) != 0) || (strcmp(ip, g_rmt_runtime_cfg.net_ip) != 0) || (g_svr_info.status == SVR_STAT_STOP)) {
                // While need to restart interface, change the status to make sure no one can call API again.
                if (g_svr_info.status == SVR_STAT_RUNNING) {
                    g_svr_info.status = SVR_STAT_STOP;
                    RMT_LOG("Stop the server for interface change.\n");
                }
                // Make sure all the API exist and no file is transfering.
                if ((g_svr_info.using_api != 0) || (dataserver_is_file_transfering())) {
                    continue;
                }
                // Make sure the available interface exist.
                if (strlen(interface) == 0) {
                    continue;
                }
                RMT_LOG("Interface %s with IP %s changed! Reinit communication...\n", g_rmt_runtime_cfg.net_interface, g_rmt_runtime_cfg.net_ip);
                if (g_svr_info.transport) {
                    RMT_LOG("Free the communication resource\n");
                    dds_transport_deinit(g_svr_info.transport);
                    g_svr_info.transport = NULL;
                    // clear the remaining device
                    devinfo_server_deinit();
                }
                if (dds_transport_config_init(interface, g_rmt_cfg.domain_id) < 0) {
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
                RMT_LOG("Init interface %s with IP %s successfully!\n", interface, ip);
                strcpy(g_rmt_runtime_cfg.net_interface, interface);
                strcpy(g_rmt_runtime_cfg.net_ip, ip);
                g_svr_info.status = SVR_STAT_RUNNING;
            }
        }
        usleep(10000); // sleep 10ms
    }
    RMT_LOG("Stop recv thread.\n")
    pthread_exit(NULL); // leave the thread
}

int rmt_server_configure(char *interface, int domain_id)
{
    rmt_config_init();
    if (interface != NULL) {
        strcpy(g_rmt_cfg.net_interface, interface);
    }
    g_rmt_cfg.domain_id = domain_id;
    return 0;
}

int rmt_server_init(void)
{
    rmt_config_init();
    rmt_config_runtime_init();
    log_init();
    rmt_config_print();
    dds_transport_config_init(g_rmt_runtime_cfg.net_interface, g_rmt_cfg.domain_id);
    devinfo_server_init();
    datainfo_server_init();
#ifdef SUPPORT_ZENOH
    if (g_rmt_cfg.support_zenoh) {
        set_robot_id_delete_callback(devinfo_server_del_device_callback_robot_id);
    }
#endif
    g_svr_info.transport = dds_transport_server_init(devinfo_server_del_device_callback);
    if (g_svr_info.transport) {
        RMT_LOG("Init server successfully\n");
        g_svr_info.recv_thread_status = 1;
        pthread_create(&g_svr_info.recv_thread, NULL, recv_thread_func, NULL);
        g_svr_info.using_api = 0;
        g_svr_info.status = SVR_STAT_RUNNING;
        return 0;
    } else {
        RMT_ERROR("Unable to init server\n");
        return -1;
    }
}

device_info* rmt_server_create_device_list(int *num)
{
    device_info *dev;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        *num = 0;
        return NULL;
    }
    g_svr_info.using_api++;

    RMT_LOG("Create device list.\n");
    devinfo_server_create_list(g_svr_info.transport, &dev, (unsigned int *)num);

    g_svr_info.using_api--;
    return dev;
}

int rmt_server_free_device_list(device_info *dev)
{
    RMT_LOG("Free device list.\n");
    return devinfo_server_free_list(dev);
}

data_info* rmt_server_get_info(unsigned long *id_list, int id_num, char *key_list, int *info_num)
{
    data_info *data;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        *info_num = 0;
        return NULL;
    }
    g_svr_info.using_api++;

    RMT_LOG("Get info.\n");
    data = datainfo_server_get_info(g_svr_info.transport, id_list, id_num, key_list, info_num);

    g_svr_info.using_api--;
    return data;
}

int rmt_server_free_info(data_info* info_list)
{
    RMT_LOG("Free info.\n");
    return datainfo_server_free_info(info_list);
}

data_info* rmt_server_set_info(data_info *dev_list, int dev_num, int *info_num)
{
    data_info *data;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        *info_num = 0;
        return NULL;
    }
    g_svr_info.using_api++;

    RMT_LOG("Set info.\n");
    data = datainfo_server_set_info(g_svr_info.transport, dev_list, dev_num, info_num);

    g_svr_info.using_api--;
    return data;
}

data_info* rmt_server_set_info_with_same_value(unsigned long *id_list, int id_num, char *value_list, int *info_num)
{
    data_info *data;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        *info_num = 0;
        return NULL;
    }
    g_svr_info.using_api++;

    RMT_LOG("Set info with same value.\n");
    data = datainfo_server_set_info_with_same_value(g_svr_info.transport, id_list, id_num, value_list, info_num);

    g_svr_info.using_api--;
    return data;
}

int rmt_server_send_file(unsigned long *id_list, int id_num, char *callbackname, char *filename, void *pFile, unsigned long file_len)
{
    int ret;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        return -1;
    }
    g_svr_info.using_api++;

    RMT_LOG("Send file.\n");
    ret = datainfo_server_send_file(g_svr_info.transport, id_list, id_num, callbackname, filename, pFile, file_len);

    g_svr_info.using_api--;
    return ret;
}

int rmt_server_recv_file(unsigned long id, char *callbackname, char *filename)
{
    int ret;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        return -1;
    }
    g_svr_info.using_api++;

    RMT_LOG("Receive file.\n");
    ret = datainfo_server_recv_file(g_svr_info.transport, id, callbackname, filename);

    g_svr_info.using_api--;
    return ret;
}

transfer_status rmt_server_get_result(unsigned long device_id, transfer_result *result)
{
    transfer_status current_status;

    if (g_svr_info.status != SVR_STAT_RUNNING) {
        return STATUS_SERVER_ERROR;
    }
    g_svr_info.using_api++;

    RMT_LOG("Get result.\n");
    devinfo_server_get_status_by_id(device_id, &current_status, result);

    g_svr_info.using_api--;
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
    log_deinit();
    rmt_config_deinit();

    return ret;
}

char* rmt_server_version(void)
{
    RMT_LOG("Get RMT version.\n");
    return PROJECT_VERSION;
}

void rmt_reinit_server(void)
{
    RMT_LOG("Reinit server.\n")
    g_svr_info.status = SVR_STAT_STOP;
    return;
}
