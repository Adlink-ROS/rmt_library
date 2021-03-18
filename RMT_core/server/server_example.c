#include <stdio.h>
#include "rmt_server.h"

int main(void)
{
    printf("RMT Library version is %s\n", rmt_server_version());
    rmt_server_init();

    return 0;
}