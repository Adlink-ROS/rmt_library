#include "dds_transport.h"
#include "datainfo_agent.h"
#include "DataInfo.h"

// RMT_TODO: This should be a queue
DataInfo_Reply datainfo_reply;

static int recv_request(void *msg)
{
    DataInfo_Request *datainfo_msg = (DataInfo_Request *) msg;

    printf("key_list: %s\n", datainfo_msg->msg);
    printf("type: %d\n", datainfo_msg->type);
    printf("id_list.length: %d\n", datainfo_msg->id_list._length);
    // RMT_TODO: check whether the ID matches or not
    for (int i = 0; i < datainfo_msg->id_list._length; i++) {
        printf("id_list.id %d:%lu\n", i, datainfo_msg->id_list._buffer[i]);
    }

    // RMT_TODO: need to parse the type
    if (datainfo_msg->type != DataInfo_GET)
        return 1;

    // RMT_TODO: need to fill out the corect reply message
    datainfo_reply.type = DataInfo_GET;
    datainfo_reply.deviceID = 1;
    datainfo_reply.msg = "cpu:20";

    return 0;
}

static int datainfo_agent_send_data(struct dds_transport *transport)
{
    // reply the data
    dds_transport_send(PAIR_DATA_REPLY, transport, &datainfo_reply);
    return 0;
}

int datainfo_agent_update(struct dds_transport *transport)
{
    dds_transport_try_recv(PAIR_DATA_REQ, transport, recv_request);
    datainfo_agent_send_data(transport);
    return 0;
}