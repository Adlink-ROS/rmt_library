#include <stdio.h>
#include "rmt_server.h"

int main(void)
{
    uint32_t dev_num;
    device_info *dev_ptr;

    printf("RMT Library version is %s\n", rmt_server_version());
    rmt_server_init();
    dev_ptr = rmt_server_create_device_list(&dev_num);
    for (int i = 0; i < dev_num; i++) {
        printf("Device %d\n", i);
        printf("ID: %d\n", dev_ptr[i].deviceID);
        printf("Model: %s\n", dev_ptr[i].model);
        printf("Host: %s\n", dev_ptr[i].host);
        printf("IP: %s\n", dev_ptr[i].ip);
        printf("MAC: %s\n", dev_ptr[i].mac);
        printf("RMT version: %s\n", dev_ptr[i].rmt_version);
        fflush (stdout);
    }
    rmt_server_free_device_list(&dev_ptr);
    rmt_server_deinit();

    return 0;
}