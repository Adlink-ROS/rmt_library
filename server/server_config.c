#include <string.h>
#include "rmt_server.h"
#include "server_config.h"
#include "network.h"
#include "logger.h"

server_config g_server_cfg;

int server_config_set(char *interface, int domain_id)
{
    int ret = 0;

    g_server_cfg.domain_id = 0;
    if (net_select_interface(g_server_cfg.net_interface) < 0) {
        RMT_ERROR("Unable to select interface.\n");
        ret = -1;
        goto exit;
    }

    // if there is user's config
    if (domain_id != 0) {
        g_server_cfg.domain_id = domain_id;
    }
    if (interface != NULL && strlen(interface) != 0) {
        strncpy(g_server_cfg.net_interface, interface, sizeof(g_server_cfg.net_interface) - 1);
        g_server_cfg.auto_detect_interface = 0;
    } else {
        // If user doesn't assign interface, we can detect interface change automatically.
        g_server_cfg.auto_detect_interface = 1;
    }

    RMT_LOG("g_agent_cfg.domain_id %d\n", g_server_cfg.domain_id);
    RMT_LOG("g_agent_cfg.net_interface %s\n", g_server_cfg.net_interface);

exit:
    return ret;
}
