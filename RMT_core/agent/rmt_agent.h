#ifndef _RMT_AGENT_
#define _RMT_AGENT_

int rmt_agent_init(char *interface, int id);
int rmt_agent_running(void);
int rmt_agent_deinit(void);
char* rmt_agent_version(void);

#endif /*_RMT_AGENT_*/