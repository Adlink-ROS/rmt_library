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

int rmt_agent_config(char *interface, int id);
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
