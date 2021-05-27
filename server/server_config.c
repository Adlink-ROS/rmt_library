#include <string.h>
#include "rmt_server.h"
#include "server_config.h"
#include "network.h"
#include "logger.h"

server_config g_server_cfg;

int server_config_set(rmt_server_cfg *config)
{
    int ret = 0;

    g_server_cfg.domain_id = 0;
    if (net_select_interface(g_server_cfg.net_interface) < 0) {
        RMT_ERROR("Unable to select interface.\n");
        ret = -1;
        goto exit;
    }

    // if there is user's config
    if (config != NULL) {
        if (config->domain_id != 0) {
            g_server_cfg.domain_id = config->domain_id;
        }
        if (config->net_interface != NULL) {
            strncpy(g_server_cfg.net_interface, config->net_interface, sizeof(g_server_cfg.net_interface));
        }
    }

exit:
    return ret;
}
