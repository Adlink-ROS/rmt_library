#ifndef _DEVICE_INFO_
#define _DEVICE_INFO_

int devinfo_server_config(char *interface);
int devinfo_server_init(void);
int devinfo_server_create_list(device_info **dev, uint32_t *num);
int devinfo_server_free_list(device_info **dev);
int devinfo_server_deinit(void);

#endif /*_DEVICE_INFO_*/