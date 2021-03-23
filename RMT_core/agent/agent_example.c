#include <stdio.h>
#include "rmt_agent.h"
#include <unistd.h> // sleep

int main(void)
{
    printf("This is RMT Agent.\n");
    rmt_agent_init();
    while (1) {
        rmt_agent_running();
        sleep(1);
    }
    rmt_agent_deinit();
    return 0;
}