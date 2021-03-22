#include "rmt_server.h"
#include "config.h"
#include "device_info.h"

int rmt_server_init(void)
{
    return device_info_subscriber_init();
}

int rmt_server_list_device(void)
{
    return list_device_info();
}

int rmt_server_get_info(void)
{
    return 0;
}

int rmt_server_set_info(void)
{
    return 0;
}

int rmt_server_deinit(void)
{
    return device_info_subscriber_deinit();
}

char* rmt_server_version(void)
{
    return PROJECT_VERSION;
}