#ifndef _DDS_TRANSPORT_
#define _DDS_TRANSPORT_

#include <stdint.h>

typedef enum _PAIR_KIND {
    PAIR_DEV_INFO,   // 0
    PAIR_DATA_REQ,   // 1
    PAIR_DATA_REPLY, // 2
    PAIR_TOTAL       // 3
} PAIR_KIND;

struct DeviceInfo_Msg;
struct dds_transport;

#ifdef SUPPORT_ZENOH
void set_robot_id_delete_callback(int (*robot_id_delete_callback)(char *));
#endif
int dds_transport_config_init(char *interface, int domain_id);
struct dds_transport *dds_transport_server_init(int (*dev_delete_callback)(uint64_t));
struct dds_transport *dds_transport_agent_init(void);
int dds_transport_send(PAIR_KIND kind, struct dds_transport *transport, void *msg);
int dds_transport_try_recv(PAIR_KIND kind, struct dds_transport *transport, int (*func)(void *, void *, void *), void *);
int dds_transport_try_recv_instance(void *instance, PAIR_KIND kind, struct dds_transport *transport, int (*func)(void *, void *, void *), void *arg);
int dds_transport_deinit(struct dds_transport *transport);

#endif /*_DDS_TRANSPORT_*/
