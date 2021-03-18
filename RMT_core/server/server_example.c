#include <stdio.h>
#include "rmt_server.h"

int main(void)
{
    printf("RMT Library version is %s\n", rmt_server_version());
    rmt_server_init();
    rmt_server_list_device();
    rmt_server_deinit();

    return 0;
}