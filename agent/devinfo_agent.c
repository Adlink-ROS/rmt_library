#include <string.h>
#include "DeviceInfo.h"
#include "dds/dds.h"
#include "dds_transport.h"
#include "version.h"
#include "network.h"
#include "logger.h"

static DeviceInfo_Msg g_msg;
static int g_info_change = 1;
typedef struct _device_info {
    char hostname[1024];
    char interface[40];
    char ip[40];
    char mac[20];
} device_info;
static device_info g_dev;

static int device_info_publisher_update(void);

static void get_device_info(void)
{
    device_info tmp_dev = g_dev;

    // Check hostname
    gethostname(tmp_dev.hostname, sizeof(tmp_dev.hostname));
    g_dev.hostname[sizeof(tmp_dev.hostname) - 1] = 0;
    // Get IP
    net_get_ip(tmp_dev.interface, tmp_dev.ip, sizeof(tmp_dev.ip));
    // Get MAC
    net_get_mac(tmp_dev.interface, tmp_dev.mac, sizeof(tmp_dev.mac));

    if (memcmp(&g_dev, &tmp_dev, sizeof(device_info)) != 0) {
        g_dev = tmp_dev;
        g_info_change = 1;
    }

    g_msg.model = "ROScube-I"; // RMT_TODO: get the correct model.
    g_msg.host = g_dev.hostname;
    g_msg.ip = g_dev.ip;
    g_msg.mac = g_dev.mac;
    g_msg.rmt_version = PROJECT_VERSION;
}

int devinfo_agent_config(char *interface, uint64_t id)
{
    int ret = 0;

    if (interface != NULL) {
        strcpy(g_dev.interface, interface);
    }
    g_msg.deviceID = id;

exit:
    return ret;
}

int devinfo_agent_update(struct dds_transport *transport)
{
    int ret = 0;

    get_device_info();

    /* If information changes */
    if (g_info_change) {
        dds_transport_send(PAIR_DEV_INFO, transport, &g_msg);
        g_info_change = 0;
    }

    return ret;
}

unsigned long devinfo_get_id(void)
{
    return g_msg.deviceID;
}
