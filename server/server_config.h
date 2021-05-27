#ifndef _SERVER_CONFIG_
#define _SERVER_CONFIG_

#include "rmt_server.h"

typedef struct _server_config {
    char net_interface[40];        // empty for default interface chosen by RMT server
    int domain_id;                 // 0 for default domain ID 0
} server_config;

extern server_config g_server_cfg;

int server_config_set(char *interface, int domain_id);

#endif /*_SERVER_CONFIG_*/
