#include "dds_transport.h"
#include "devinfo_agent.h"
#include "datainfo_agent.h"
#include "DataInfo.h"
#include "logger.h"

// RMT_TODO: This should be a queue
DataInfo_Reply datainfo_reply;

static int recv_request(void *msg)
{
    unsigned long myid = devinfo_get_id();
    DataInfo_Request *datainfo_msg = (DataInfo_Request *) msg;

    RMT_LOG("key_list: %s\n", datainfo_msg->msg);
    RMT_LOG("type: %d\n", datainfo_msg->type);
    RMT_LOG("id_list.length: %d\n", datainfo_msg->id_list._length);
    // check whether the ID matches or not
    int dev_found = 0;
    for (int i = 0; i < datainfo_msg->id_list._length; i++) {
        RMT_LOG("id_list.id %d:%lu\n", i, datainfo_msg->id_list._buffer[i]);
        if (myid == datainfo_msg->id_list._buffer[i]) {
            RMT_LOG("The ID matches!\n");
            dev_found = 1;
            break;
        }
    }
    // The request is not for me.
    if (!dev_found) return 1;

    if (datainfo_msg->type == DataInfo_GET) {
        // return the get info back
        datainfo_reply.type = DataInfo_GET;
        datainfo_reply.deviceID = myid;
        // parse keylist and the return with value
        // RMT_TODO: need to fill out the corect reply message
        // RMT_TODO: we need to able to pass the handling function into here
        datainfo_reply.msg = "cpu:20";
    } else if (datainfo_msg->type == DataInfo_SET) {
        // set the info
    } else {
        // wrong type
        return 1;
    }

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