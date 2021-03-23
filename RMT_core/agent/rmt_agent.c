#include "rmt_agent.h"
#include "devinfo_agent.h"
#include "config.h"

int rmt_agent_init(void)
{
    return devinfo_agent_init();
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