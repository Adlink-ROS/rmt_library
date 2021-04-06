#include "rmt_agent.h"
#include "dds_transport.h"
#include "devinfo_agent.h"
#include "datainfo_agent.h"
#include "version.h"
#include "logger.h"

static struct dds_transport *g_transport;

int rmt_agent_config(char *interface, int id)
{
    dds_transport_config_init(interface);
    devinfo_agent_config(interface, id);
}

int rmt_agent_init(datainfo_func *func_maps)
{
    datainfo_agent_init(func_maps);
    g_transport = dds_transport_agent_init();
    if (g_transport) {
        return 0;
    } else {
        RMT_ERROR("Unable to init agent\n");
        return -1;
    }
}

// RMT_TODO: users can add their own config
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