#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "rmt_agent.h"
#include "agent_config.h"
#include "network.h"
#include "logger.h"

agent_config g_agent_cfg;

int agent_config_set(rmt_agent_cfg *config)
{
    int ret = 0;

    // Setup default config
    g_agent_cfg.domain_id = 0;
    g_agent_cfg.datainfo_val_size = 256;
    g_agent_cfg.devinfo_size = 1024;
    if (net_select_interface(g_agent_cfg.net_interface) < 0) {
        RMT_ERROR("Unable to select interface.\n");
        ret = -1;
        goto exit;
    }
    g_agent_cfg.device_id = net_get_id_from_mac(g_agent_cfg.net_interface);
    g_agent_cfg.user_config = NULL;

    // If there is user's config
    if (config != NULL) {
        g_agent_cfg.user_config = malloc(sizeof(rmt_agent_cfg));
        *g_agent_cfg.user_config = *config;
        if (config->domain_id != 0) {
            g_agent_cfg.domain_id = config->domain_id;
        }
        if (config->datainfo_val_size != 0) {
            g_agent_cfg.datainfo_val_size = config->datainfo_val_size;
        }
        if (config->device_id != 0) {
            g_agent_cfg.device_id = config->device_id;
        }
        if (config->net_interface != NULL) {
            strncpy(g_agent_cfg.net_interface, config->net_interface, sizeof(g_agent_cfg.net_interface) - 1);
            g_agent_cfg.user_config->net_interface = strdup(config->net_interface);
        }
        if (config->devinfo_size != 0) {
            g_agent_cfg.devinfo_size = config->devinfo_size;
        }
    }

    RMT_LOG("g_agent_cfg.domain_id %d\n", g_agent_cfg.domain_id);
    RMT_LOG("g_agent_cfg.datainfo_val_size %lu\n", g_agent_cfg.datainfo_val_size);
    RMT_LOG("g_agent_cfg.devinfo_size %lu\n", g_agent_cfg.devinfo_size);
    RMT_LOG("g_agent_cfg.device_id %lu\n", g_agent_cfg.device_id);
    RMT_LOG("g_agent_cfg.net_interface %s\n", g_agent_cfg.net_interface);

exit:
    return ret;
}

void agent_config_deinit(void)
{
    if (g_agent_cfg.user_config) {
        if (g_agent_cfg.user_config->net_interface) {
            free(g_agent_cfg.user_config->net_interface);
            g_agent_cfg.user_config->net_interface = NULL;
        }
        free(g_agent_cfg.user_config);
        g_agent_cfg.user_config = NULL;
    }
}
