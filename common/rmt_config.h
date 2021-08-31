#ifndef _CONFIG_
#define _CONFIG_

#include <stdint.h>

/* The config read from file */
typedef struct _rmt_config {
    char net_interface[40];          // The default network interface used by RMT
    int domain_id;                   // The current domain ID
    int auto_detect_interface;       // Auto switch the interface
    char logfile[40];                // Where to put the log
    int reply_timeout;               // Only used by server: set the timeout for receiving reply message from agents.
    uint64_t device_id;              // Only used by agent: Agent ID. 0 means it will be auto generated.
    unsigned long datainfo_val_size; // Only used by agent: Buffer size for datainfo
    unsigned long devinfo_size;      // Only used by agent: Buffer size for devinfo
#ifdef SUPPORT_ZENOH
    int support_zenoh;               // Able to use zenoh or not
#endif /*SUPPORT_ZENOH*/
} rmt_config;

extern rmt_config g_rmt_cfg;

/* The runtime config */
typedef struct _rmt_runtime_cfg {
    char net_interface[40];
    char net_ip[40];
} rmt_runtime_cfg;

extern rmt_runtime_cfg g_rmt_runtime_cfg;

void rmt_config_print(void);
void rmt_config_init(void);
void rmt_config_runtime_init(void);
void rmt_config_deinit(void);

#endif /*_CONFIG_*/
