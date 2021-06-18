#ifndef _DEVICEINFO_SERVER_H_
#define _DEVICEINFO_SERVER_H_

#include <stdint.h>

extern transfer_result empty_result;

int devinfo_server_del_device_callback(uint64_t internal_id);
int devinfo_server_update(struct dds_transport *transport);
int devinfo_server_create_list(struct dds_transport *transport, device_info **dev, uint32_t *num);
int devinfo_server_free_list(device_info *dev);
void devinfo_server_init(void);
void devinfo_server_deinit(void);
int devinfo_server_set_status_by_id(unsigned long id, transfer_status dev_status, transfer_result dev_result);
int devinfo_server_get_status_by_id(unsigned long id, transfer_status *dev_status, transfer_result *dev_result);

#endif /*_DEVICEINFO_SERVER_H_*/
