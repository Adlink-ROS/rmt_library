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

    g_agent_cfg.domain_id = 0;
    g_agent_cfg.datainfo_val_size = 256;
    if (net_select_interface(g_agent_cfg.net_interface) < 0) {
        RMT_ERROR("Unable to select interface.\n");
        ret = -1;
        goto exit;
    }
    g_agent_cfg.device_id = net_get_id_from_mac(g_agent_cfg.net_interface);

    // if there is user's config
    if (config != NULL) {
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
            strncpy(g_agent_cfg.net_interface, config->net_interface, sizeof(g_agent_cfg.net_interface));
        }
    }

    RMT_LOG("g_agent_cfg.domain_id %d\n", g_agent_cfg.domain_id);
    RMT_LOG("g_agent_cfg.datainfo_val_size %d\n", g_agent_cfg.datainfo_val_size);
    RMT_LOG("g_agent_cfg.device_id %lu\n", g_agent_cfg.device_id);
    RMT_LOG("g_agent_cfg.net_interface %s\n", g_agent_cfg.net_interface);

exit:
    return ret;
}
