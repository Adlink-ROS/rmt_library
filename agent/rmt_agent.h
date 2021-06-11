#ifndef _RMT_AGENT_
#define _RMT_AGENT_

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

#include <stdint.h>

/**
 * @file
 * RMT agent library API.
 */

typedef void (*devinfo_func)(char *);
typedef int (*INFO_FUNC)(char *);

typedef struct datainfo_func {
    const char *key;
    INFO_FUNC get_func;
    INFO_FUNC set_func;
} datainfo_func;

typedef int (*FILE_FUNC)(char *, char *, int);

typedef struct fileinfo_func {
    const char *callbackname;
    const char *path;
    FILE_FUNC import_func;
    FILE_FUNC export_func;
} fileinfo_func;

typedef struct _rmt_agent_cfg {
    char *net_interface;             // NULL for default interface chosen by RMT agent
    uint64_t device_id;              // 0 for default device ID generated from MAC
    int domain_id;                   // 0 for default domain ID 0
    unsigned long datainfo_val_size; // 0 for default size 256
    unsigned long devinfo_size;      // 0 for default size 1024
} rmt_agent_cfg;

/**
 * @brief rmt_agent_configure
 * Set the config for RMT agent
 *
 * @param[in] rmt_agent_cfg The config structure for RMT agent
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_agent_configure(rmt_agent_cfg *config);

/**
 * @brief rmt_agent_init
 * Init RMT agent
 *
 * @param[in] search_func The callback for getting devinfo
 * @param[in] func_maps The function map for getting datainfo
 * @param[in] file_maps The function map for getting fileinfo
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_agent_init(devinfo_func agent_devinfo_func, datainfo_func *data_func_maps, fileinfo_func *file_func_maps);

/**
 * @brief rmt_agent_running
 * Update the device data and receive requests from server
 * Note: the function should always run
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_agent_running(void);

/**
 * @brief rmt_agent_deinit
 * Release resource of RMT agent
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_agent_deinit(void);

/**
 * @brief rmt_agent_version
 * Get the current version of RMT agent library
 *
 * @returns The version of RMT agent library
 */
char* rmt_agent_version(void);

/* *INDENT-OFF* */
#ifdef __cplusplus
} /* extern "C" */
#endif 
/* *INDENT-ON* */

#endif /*_RMT_AGENT_*/
