#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"
#include "rmt_config.h"
#include "network.h"

#define LINE_LEN 1024

/* global variable for config */
rmt_config g_rmt_cfg;
rmt_runtime_cfg g_rmt_runtime_cfg;

/* *INDENT-OFF* */
/* The uncrustify in Ubuntu 18.04 can't handle enum indent well */
/* config init status */
typedef enum _rmt_config_status {
    RMT_CFG_NOT_INIT = 0,
    RMT_CFG_INIT     = 1
} rmt_config_status;
/* *INDENT-ON* */

static rmt_config_status g_rmt_cfg_status = RMT_CFG_NOT_INIT;

/* *INDENT-OFF* */
/* The uncrustify in Ubuntu 18.04 can't handle enum indent well */
/* config type and mapping */
typedef enum _config_type {
    CONFIG_NONE   = 0,
    CONFIG_INT    = 1,
    CONFIG_STRING = 2
} config_type;
/* *INDENT-ON* */

typedef struct _config_mapping {
    char *name;
    config_type type;
    void *pointer;
} config_mapping;

config_mapping g_config_mapping[] = {
    { "interface",        CONFIG_STRING, g_rmt_cfg.net_interface          },
    { "domain",           CONFIG_INT,    &g_rmt_cfg.domain_id             },
    { "switch_interface", CONFIG_INT,    &g_rmt_cfg.auto_detect_interface },
    { "logfile",          CONFIG_STRING, g_rmt_cfg.logfile                },
    { "reply_timeout",    CONFIG_INT,    &g_rmt_cfg.reply_timeout         },
    { "device_id",        CONFIG_INT,    &g_rmt_cfg.device_id             },
    { "datainfo_size",    CONFIG_INT,    &g_rmt_cfg.datainfo_val_size     },
    { "devinfo_size",     CONFIG_INT,    &g_rmt_cfg.devinfo_size          },
    { "keepalive_time",   CONFIG_INT,    &g_rmt_cfg.keepalive_time        },
#ifdef SUPPORT_ZENOH
    { "support_zenoh",    CONFIG_INT,    &g_rmt_cfg.support_zenoh         },
#endif /*SUPPORT_ZENOH*/
    { NULL,               CONFIG_NONE,   NULL                             }
};

static void init_rmt_cfg(void)
{
    /* Setup default config */
    memset(&g_rmt_cfg, 0, sizeof(g_rmt_cfg));
    g_rmt_cfg.net_interface[0] = '\0';
    g_rmt_cfg.domain_id = 0;
    g_rmt_cfg.auto_detect_interface = 1;
    strcpy(g_rmt_cfg.logfile, "stderr");
    g_rmt_cfg.reply_timeout = 3;
    g_rmt_cfg.device_id = 0;
    g_rmt_cfg.datainfo_val_size = 256;
    g_rmt_cfg.devinfo_size = 1024;
    g_rmt_cfg.keepalive_time = 5;
#ifdef SUPPORT_ZENOH
    g_rmt_cfg.support_zenoh = 0;
#endif /*SUPPORT_ZENOH*/
}

void rmt_config_print(void)
{
    for (int i = 0; g_config_mapping[i].name != NULL; i++) {
        switch (g_config_mapping[i].type) {
            case CONFIG_STRING:
                RMT_LOG("%s: %s\n", g_config_mapping[i].name, (char *)g_config_mapping[i].pointer);
                break;
            case CONFIG_INT:
                RMT_LOG("%s: %d\n", g_config_mapping[i].name, *((int *)g_config_mapping[i].pointer));
                break;
            default:
                break;
        }
    }
}

void rmt_config_init(void)
{
    char *config_path;

    /* Already init */
    if (g_rmt_cfg_status != RMT_CFG_NOT_INIT) {
        return;
    }

    // Clear rmt_cfg
    init_rmt_cfg();

    // Get environment variables
    config_path = getenv("RMT_CONFIG");
    if (config_path == NULL) {
        config_path = "./rmt.conf";
    }

    char line[LINE_LEN];
    int line_num = 0;
    char key[128];
    char value[256];
    FILE *fp = fopen(config_path, "r");
    if (fp == NULL) {
        RMT_ERROR("Unable to find config, such as rmt.conf.\n");
        goto exit;
    }
    while (fgets(line, LINE_LEN, fp) != NULL) {
        line_num++;
        // This is comment
        if ((line[0] == '#') || (line[0] == '\n')) continue;
        // Parse key&value
        if (sscanf(line, "%[^:]:%s", key, value) != 2) {
            RMT_ERROR("Syntax error: line %d\n", line_num);
            continue;
        }
        //RMT_LOG("Line %d: key=%s, value=%s\n", line_num, key, value);  // debug use
        // Put legal key&value into rmt_cfg
        for (int i = 0; g_config_mapping[i].name != NULL; i++) {
            if (strcmp(g_config_mapping[i].name, key) == 0) {
                switch (g_config_mapping[i].type) {
                    case CONFIG_STRING:
                        strcpy(g_config_mapping[i].pointer, value);
                        break;
                    case CONFIG_INT:
                        *((int *) g_config_mapping[i].pointer) = atoi(value);
                        break;
                    default:
                        break;
                }
                break;
            }
        }
    }

    fclose(fp);

exit:
    g_rmt_cfg_status = RMT_CFG_INIT;
    return;
}

void rmt_config_runtime_init(void)
{
    /* Setup runtime interface */
    if (strlen(g_rmt_cfg.net_interface) != 0) {
        strcpy(g_rmt_runtime_cfg.net_interface, g_rmt_cfg.net_interface);
    } else {
        /* If there is no config for network interface, detect by ourselves */
        if (net_select_interface(g_rmt_runtime_cfg.net_interface) < 0) {
            //RMT_ERROR("Unable to select interface.\n");
            return;
        }
    }

    /* Setup runtime iP */
    if (net_get_ip(g_rmt_runtime_cfg.net_interface, g_rmt_runtime_cfg.net_ip, sizeof(g_rmt_runtime_cfg.net_ip)) < 0) {
        //RMT_ERROR("Unable to get IP from interface %s\n", g_rmt_runtime_cfg.net_interface);
        return;
    }

    /* Auto generated device ID */
    if (g_rmt_cfg.device_id == 0) {
        g_rmt_cfg.device_id = net_generate_id();
    }

    return;
}

void rmt_config_deinit(void)
{
    g_rmt_cfg_status = RMT_CFG_NOT_INIT;
}
