#include <string.h>
#include "rmt_agent.h"
#include "dds_transport.h"
#include "devinfo_agent.h"
#include "datainfo_agent.h"
#include "version.h"
#include "logger.h"
#include "agent_config.h"
#include "network.h"
#include "config.h"

static struct dds_transport *g_transport;

// RMT_TODO: I think it would be better to use string with key-value instead of structure
//           Because we can decide whether to keep some config default or not.
int rmt_agent_configure(rmt_agent_cfg *config)
{
    log_init();
    return agent_config_set(config);
}

int rmt_agent_init(devinfo_func agent_devinfo_func, datainfo_func *data_func_maps, fileinfo_func *file_func_maps)
{
    int ret = 0;

    rmt_config_init();
    devinfo_agent_init(agent_devinfo_func);
    datainfo_agent_init(data_func_maps, file_func_maps, g_agent_cfg.datainfo_val_size);
    if (dds_transport_config_init(g_agent_cfg.net_interface, g_agent_cfg.domain_id) < 0) {
        RMT_ERROR("Unable to init communication\n");
        ret = -1;
        goto exit;
    }
    g_transport = dds_transport_agent_init();
    if (!g_transport) {
        RMT_ERROR("Unable to init agent\n");
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

int rmt_agent_running(void)
{
    if ((g_agent_cfg.user_config == NULL) || (g_agent_cfg.user_config->net_interface == NULL)) {
        char interface[40] = {0};
        char ip[40] = {0};

        net_select_interface(interface);
        net_get_ip(interface, ip, sizeof(ip));
        if ((strcmp(interface, g_agent_cfg.net_interface) != 0) || (strcmp(ip, g_agent_cfg.net_ip) != 0)) {
            RMT_LOG("Interface %s with IP %s changed! Reinit communication...\n", g_agent_cfg.net_interface, g_agent_cfg.net_ip);
            if (g_transport) {
                RMT_LOG("Free the communication resource\n");
                dds_transport_deinit(g_transport);
                g_transport = NULL;
            }
            if (dds_transport_config_init(interface, g_agent_cfg.domain_id) < 0) {
                RMT_ERROR("Unable to init communication\n");
                return -1;
            }
            g_transport = dds_transport_agent_init();
            if (g_transport == NULL) {
                RMT_ERROR("Unable to init agent\n");
                return -1;
            }
            RMT_LOG("Init interface %s with IP %s successfully!\n", interface, ip);
            strcpy(g_agent_cfg.net_interface, interface);
            strcpy(g_agent_cfg.net_ip, ip);
        }
    }
    devinfo_agent_update(g_transport);
    datainfo_agent_update(g_transport);
    return 0;
}

int rmt_agent_deinit(void)
{
    int ret;

    devinfo_agent_deinit();
    ret = dds_transport_deinit(g_transport);
    agent_config_deinit();
    log_deinit();

    return ret;
}

char* rmt_agent_version(void)
{
    return PROJECT_VERSION;
}
