#ifndef _DDS_TRANSPORT_
#define _DDS_TRANSPORT_

struct DeviceInfo_Msg;

int dds_transport_domain_init(char *interface);
int dds_transport_server_init(void);
int dds_transport_agent_init(void);
int dds_transport_send_devinfo(struct DeviceInfo_Msg *msg);
int dds_transport_try_get_devinfo(int (*func)(struct DeviceInfo_Msg *));
int dds_transport_deinit(void);

#endif /*_DDS_TRANSPORT_*/