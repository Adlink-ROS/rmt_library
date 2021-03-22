#ifndef _DEVICE_INFO_
#define _DEVICE_INFO_

int device_info_subscriber_init(void);
int list_device_info(device_info **dev, uint32_t *num);
int device_info_subscriber_deinit(void);

#endif /*_DEVICE_INFO_*/