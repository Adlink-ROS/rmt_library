#include "rmt_agent.h"
#include "device_info.h"
#include <unistd.h> // sleep

int rmt_agent_init(void)
{
    return device_info_init();
}

int rmt_agent_running(void)
{
    while (1) {
        device_info_update();
        sleep(1);
    }
    return 0;
}

int rmt_agent_deinit(void)
{
    return device_info_deinit();
}