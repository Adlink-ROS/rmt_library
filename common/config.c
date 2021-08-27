#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

#define LINE_LEN 1024
#define DEBUG    0

/* global variable for config */
rmt_config g_rmt_cfg;
rmt_runtime_cfg g_rmt_runtime_cfg;

/* config init status */
typedef enum _rmt_config_status {
    RMT_CFG_NOT_INIT = 0,
    RMT_CFG_INIT     = 1
} rmt_config_status;

static rmt_config_status g_rmt_cfg_status = RMT_CFG_NOT_INIT;

/* config type and mapping */
typedef enum _config_type {
    CONFIG_NONE   = 0,
    CONFIG_INT    = 1,
    CONFIG_STRING = 2
} config_type;

typedef struct _config_mapping {
    char *name;
    config_type type;
    void *pointer;
} config_mapping;

config_mapping g_config_mapping[] = {
    { "interface", CONFIG_STRING, g_rmt_cfg.net_interface },
    { "domain",    CONFIG_INT,    &g_rmt_cfg.domain_id    },
    { NULL,        CONFIG_NONE,   NULL                    }
};

static void init_rmt_cfg(void)
{
    memset(&g_rmt_cfg, 0, sizeof(g_rmt_cfg));
    g_rmt_cfg.net_interface[0] = '\0';
    g_rmt_cfg.domain_id = 0;
}

#if DEBUG
static void print_rmt_cfg(void)
{
    for (int i = 0; g_config_mapping[i].name != NULL; i++) {
        switch (g_config_mapping[i].type) {
            case CONFIG_STRING:
                printf("%s: %s\n", g_config_mapping[i].name, (char *)g_config_mapping[i].pointer);
                break;
            case CONFIG_INT:
                printf("%s: %d\n", g_config_mapping[i].name, *((int *)g_config_mapping[i].pointer));
                break;
            default:
                break;
        }
    }
}

#endif /*DEBUG*/

void rmt_config_deinit(void)
{
    g_rmt_cfg_status = RMT_CFG_NOT_INIT;
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
        fprintf(stderr, "Unable to find config, such as rmt.conf.\n");
        return;
    }
    while (fgets(line, LINE_LEN, fp) != NULL) {
        line_num++;
        // This is comment
        if ((line[0] == '#') || (line[0] == '\n')) continue;
        // Parse key&value
        if (sscanf(line, "%[^:]:%s", key, value) != 2) {
            fprintf(stderr, "Syntax error: line %d\n", line_num);
            continue;
        }
        // Put legal key&value into rmt_cfg
#if DEBUG
        printf("Line %d: key=%s, value=%s\n", line_num, key, value);  // debug use
#endif
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
#if DEBUG
    print_rmt_cfg();  // debug use
#endif
    fclose(fp);

    g_rmt_cfg_status = RMT_CFG_INIT;

    return;
}
