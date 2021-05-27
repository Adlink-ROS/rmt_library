#include "rmt_agent.h"
#include "dds_transport.h"
#include "devinfo_agent.h"
#include "datainfo_agent.h"
#include "version.h"
#include "logger.h"
#include "agent_config.h"

static struct dds_transport *g_transport;

// RMT_TODO: I think it would be better to use string with key-value instead of structure
//           Because we can decide whether to keep some config default or not.
int rmt_agent_configure(rmt_agent_cfg *config)
{
    return agent_config_set(config);
}

int rmt_agent_init(datainfo_func *func_maps, fileinfo_func *file_maps)
{
    dds_transport_config_init(g_agent_cfg.net_interface);
    devinfo_agent_config(g_agent_cfg.net_interface, g_agent_cfg.device_id);
    datainfo_agent_init(func_maps, file_maps, g_agent_cfg.datainfo_val_size);
    g_transport = dds_transport_agent_init();
    if (g_transport) {
        return 0;
    } else {
        RMT_ERROR("Unable to init agent\n");
        return -1;
    }
}

int rmt_agent_running(void)
{
    devinfo_agent_update(g_transport);
    datainfo_agent_update(g_transport);
    return 0;
}

int rmt_agent_deinit(void)
{
    return dds_transport_deinit(g_transport);
}

char* rmt_agent_version(void)
{
    return PROJECT_VERSION;
}
