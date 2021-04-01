#ifndef _DATAINFO_SERVER_
#define _DATAINFO_SERVER_

data_info* datainfo_server_get_info(struct dds_transport *transport, unsigned long *id_list, char *key_list, int dev_num);

#endif /*_DATAINFO_SERVER_*/