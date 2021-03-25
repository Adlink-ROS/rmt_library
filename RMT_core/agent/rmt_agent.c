#include "rmt_agent.h"
#include "devinfo_agent.h"
#include "version.h"

int rmt_agent_init(char *interface)
{
    return devinfo_agent_init(interface);
}

int rmt_agent_running(void)
{
    return devinfo_agent_update();
}

int rmt_agent_deinit(void)
{
    return devinfo_agent_deinit();
}

char* rmt_agent_version(void)
{
    return PROJECT_VERSION;
}