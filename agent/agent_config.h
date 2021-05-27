#ifndef _AGENT_CONFIG_
#define _AGENT_CONFIG_

#include <stdint.h>
#include "rmt_agent.h"

typedef struct _agent_config {
    char net_interface[40];          // empty for default interface chosen by RMT agent
    uint64_t device_id;              // 0 for default device ID generated from MAC
    int domain_id;                   // 0 for default domain ID 0
    unsigned long datainfo_val_size; // 0 for default size 256
} agent_config;

extern agent_config g_agent_cfg;

int agent_config_set(rmt_agent_cfg *config);

#endif /*_AGENT_CONFIG_*/
