#include "rmt_agent.h"
#include "dds_transport.h"
#include "devinfo_agent.h"
#include "version.h"

int rmt_agent_config(char *interface, int id)
{
    dds_transport_domain_init(interface);
    devinfo_agent_config(interface, id);
}

int rmt_agent_init(void)
{
    return dds_transport_agent_init();
}

int rmt_agent_running(void)
{
    return devinfo_agent_update();
}

int rmt_agent_deinit(void)
{
    return dds_transport_deinit();
}

char* rmt_agent_version(void)
{
    return PROJECT_VERSION;
}