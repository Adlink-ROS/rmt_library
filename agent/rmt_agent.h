#ifndef _RMT_AGENT_
#define _RMT_AGENT_

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

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
    char *net_interface;           // NULL for default interface chosen by RMT agent
    int device_id;                 // 0 for default device ID generated from MAC
    int domain_id;                 // 0 for default domain ID 0
    unsigned long getinfo_bufsize; // 0 for default size 256
} rmt_agent_cfg;

int rmt_agent_configure(rmt_agent_cfg *config);
int rmt_agent_init(datainfo_func *func_maps, fileinfo_func *file_maps);
int rmt_agent_running(void);
int rmt_agent_deinit(void);
char* rmt_agent_version(void);

/* *INDENT-OFF* */
#ifdef __cplusplus
} /* extern "C" */
#endif 
/* *INDENT-ON* */

#endif /*_RMT_AGENT_*/
