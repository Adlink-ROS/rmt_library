#ifndef _DEVICE_INFO_
#define _DEVICE_INFO_

int device_info_init(void);
int device_info_create_list(device_info **dev, uint32_t *num);
int device_info_free_list(device_info **dev);
int device_info_deinit(void);

#endif /*_DEVICE_INFO_*/