#ifndef _DEVICEINFO_AGENT_
#define _DEVICEINFO_AGENT_

int devinfo_agent_config(char *interface, int id);
int devinfo_agent_init(void);
int devinfo_agent_update(void);
int devinfo_agent_deinit(void);

#endif /*_DEVICEINFO_AGENT_*/