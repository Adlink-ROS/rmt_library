#ifndef _CONFIG_
#define _CONFIG_

typedef struct _rmt_config {
    char net_interface[40];        // The default network interface used by RMT
    int domain_id;                 // The current domain ID
} rmt_config;

extern rmt_config g_rmt_cfg;

void rmt_config_init(void);

#endif /*_CONFIG_*/
