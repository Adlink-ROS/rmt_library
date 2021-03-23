#include "rmt_agent.h"
#include "devinfo_agent.h"
#include <unistd.h> // sleep
#include "config.h"

int rmt_agent_init(void)
{
    return devinfo_agent_init();
}

int rmt_agent_running(void)
{
    while (1) {
        devinfo_agent_update();
        sleep(1);
    }
    return 0;
}

int rmt_agent_deinit(void)
{
    return devinfo_agent_deinit();
}

char* rmt_agent_version(void)
{
    return PROJECT_VERSION;
}