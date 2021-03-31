#ifndef _DDS_TRANSPORT_
#define _DDS_TRANSPORT_

struct DeviceInfo_Msg;
struct dds_transport;

int dds_transport_config_init(char *interface);
struct dds_transport *dds_transport_server_init(void);
struct dds_transport *dds_transport_agent_init(void);
int dds_transport_send_devinfo(struct dds_transport *transport, struct DeviceInfo_Msg *msg);
int dds_transport_try_get_devinfo(struct dds_transport *transport, int (*func)(struct DeviceInfo_Msg *));
int dds_transport_deinit(struct dds_transport *transport);

#endif /*_DDS_TRANSPORT_*/