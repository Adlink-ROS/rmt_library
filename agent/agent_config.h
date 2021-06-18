#ifndef _AGENT_CONFIG_
#define _AGENT_CONFIG_

#include <stdint.h>
#include "rmt_agent.h"

typedef struct _agent_config {
    char net_interface[40];          // empty for default interface chosen by RMT agent
    uint64_t device_id;              // 0 for default device ID generated from MAC
    int domain_id;                   // 0 for default domain ID 0
    unsigned long datainfo_val_size; // 0 for default size 256
    unsigned long devinfo_size;      // 0 for default size 1024
    rmt_agent_cfg *user_config;      // backup user original config
} agent_config;

extern agent_config g_agent_cfg;

int agent_config_set(rmt_agent_cfg *config);
void agent_config_deinit(void);

#endif /*_AGENT_CONFIG_*/
