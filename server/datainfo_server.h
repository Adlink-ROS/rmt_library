#ifndef _DATAINFO_SERVER_
#define _DATAINFO_SERVER_

void datainfo_server_init(void);
data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, int id_num, char *key_list, int *info_num);
int datainfo_server_free_info(data_info* info_list);
data_info* datainfo_server_set_info(struct dds_transport *transport, data_info *dev_list, int dev_num, int *info_num);
data_info* datainfo_server_set_info_with_same_value(struct dds_transport *transport, unsigned long *id_list, int id_num, char *value_list, int *info_num);
int datainfo_server_send_file(struct dds_transport *transport, unsigned long *id_list, int id_num, char *callbackname, char *filename, void *pFile, uint32_t file_len);
int datainfo_server_recv_file(struct dds_transport *transport, unsigned long id, char *callbackname, char *filename);
void dataserver_info_file_transfer_thread(struct dds_transport *transport);
int dataserver_is_file_transfering(void);

#endif /*_DATAINFO_SERVER_*/
