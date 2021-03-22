#include <stdio.h>
#include "rmt_server.h"

int main(void)
{
    uint32_t dev_num;
    device_info *dev_ptr;

    printf("RMT Library version is %s\n", rmt_server_version());
    rmt_server_init();
    rmt_server_list_device(dev_ptr, &dev_num);
    rmt_server_deinit();

    return 0;
}