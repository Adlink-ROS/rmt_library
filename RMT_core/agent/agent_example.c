#include <stdio.h>
#include "rmt_agent.h"

int main(void)
{
    printf("This is RMT Agent.\n");
    rmt_agent_init();
    rmt_agent_running();
    rmt_agent_deinit();
    return 0;
}