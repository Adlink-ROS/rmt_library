#ifndef _DATAINFO_SERVER_
#define _DATAINFO_SERVER_

data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, int id_num, char *key_list, int *info_num);
int datainfo_server_free_info(data_info* info_list);
data_info* datainfo_server_set_info(struct dds_transport *transport, data_info *dev_list, int dev_num, int *info_num);
data_info* datainfo_server_set_info_with_same_value(struct dds_transport *transport, unsigned long *id_list, int id_num, char *value_list, int *info_num);

#endif /*_DATAINFO_SERVER_*/