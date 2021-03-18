#ifndef _RMT_SERVER_
#define _RMT_SERVER_

int rmt_server_init(void);
int rmt_server_list_device(void);
int rmt_server_deinit(void);
char* rmt_server_version(void);

#endif /*_RMT_SERVER_*/