#ifndef _DEVICEINFO_SERVER_H_
#define _DEVICEINFO_SERVER_H_

int devinfo_server_config(char *interface);
int devinfo_server_init(void);
int devinfo_server_create_list(device_info **dev, uint32_t *num);
int devinfo_server_free_list(device_info **dev);
int devinfo_server_deinit(void);

#endif /*_DEVICEINFO_SERVER_H_*/