#ifndef _DEVICEINFO_SERVER_H_
#define _DEVICEINFO_SERVER_H_

int devinfo_server_del_device_callback(long internal_id);
int devinfo_server_update(struct dds_transport *transport);
int devinfo_server_create_list(struct dds_transport *transport, device_info **dev, uint32_t *num);
int devinfo_server_free_list(device_info *dev);
int devinfo_server_deinit(void);
int devinfo_server_set_status_by_id(int id, transfer_status dev_status, transfer_result dev_result);
int devinfo_server_get_status_by_id(int id, transfer_status *dev_status, transfer_result *dev_result);

#endif /*_DEVICEINFO_SERVER_H_*/
