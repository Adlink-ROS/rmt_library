#include <stdlib.h>
#include <string.h>
#include "rmt_agent.h"
#include "DeviceInfo.h"
#include "dds/dds.h"
#include "dds_transport.h"
#include "version.h"
#include "network.h"
#include "logger.h"
#include "config.h"

static DeviceInfo_Msg g_msg;
static int g_info_change = 1;
typedef struct _device_info {
    char model[256];
    char hostname[1024];
    char ip[40];
    char mac[20];
} device_info;
static device_info g_dev;
static devinfo_func g_agent_devinfo_func = NULL;
static char *g_devinfo;

// Workaround: we don't have header file here.
int product_verifier_show_products(char *buf, int buf_size);

static void get_device_info(void)
{
    device_info tmp_dev = g_dev;
    char *tmp_devinfo = (char *) calloc(g_rmt_cfg.devinfo_size, sizeof(char));

    // Check hostname
    gethostname(tmp_dev.hostname, sizeof(tmp_dev.hostname));
    g_dev.hostname[sizeof(tmp_dev.hostname) - 1] = 0;
    // Get IP
    net_get_ip(g_rmt_runtime_cfg.net_interface, tmp_dev.ip, sizeof(tmp_dev.ip));
    // Get MAC
    net_get_mac(g_rmt_runtime_cfg.net_interface, tmp_dev.mac, sizeof(tmp_dev.mac));
    // Call customized search info
    if (g_agent_devinfo_func != NULL)
        g_agent_devinfo_func(tmp_devinfo);

    if ((memcmp(&g_dev, &tmp_dev, sizeof(device_info)) != 0)
        || (strcmp(g_devinfo, tmp_devinfo) != 0)) {
        g_dev = tmp_dev;
        memset(g_devinfo, 0, g_rmt_cfg.devinfo_size);
        strcpy(g_devinfo, tmp_devinfo);
        g_info_change = 1;
    }
    free(tmp_devinfo);

    memset(g_dev.model, 0, sizeof(g_dev.model));
    product_verifier_show_products(g_dev.model, sizeof(g_dev.model));
    g_msg.model = g_dev.model;
    g_msg.host = g_dev.hostname;
    g_msg.ip = g_dev.ip;
    g_msg.mac = g_dev.mac;
    g_msg.rmt_version = PROJECT_VERSION;
    g_msg.devinfo = g_devinfo;
}

int devinfo_agent_init(devinfo_func agent_devinfo_func)
{
    int ret = 0;

    g_msg.deviceID = g_rmt_cfg.device_id;
    g_agent_devinfo_func = agent_devinfo_func;
    g_devinfo = (char *) calloc(g_rmt_cfg.devinfo_size, sizeof(char));

    return ret;
}

void devinfo_agent_deinit(void)
{
    free(g_devinfo);
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
