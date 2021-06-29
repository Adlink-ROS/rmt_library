#ifndef _SERVER_CONFIG_
#define _SERVER_CONFIG_

#include "rmt_server.h"

typedef struct _server_config {
    char net_interface[40];        // The current interface used by RMT server
    char net_ip[40];               // The current IP used by RMT server
    int auto_detect_interface;     // 1 for interface auto detection, only enabled while user doesn't assign his/her interface
    int domain_id;                 // The current domain ID
} server_config;

extern server_config g_server_cfg;

int server_config_set(char *interface, int domain_id);

#endif /*_SERVER_CONFIG_*/
