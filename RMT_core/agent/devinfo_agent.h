#ifndef _DEVICE_INFO_
#define _DEVICE_INFO_

int devinfo_agent_config(char *interface, int id);
int devinfo_agent_init(void);
int devinfo_agent_update(void);
int devinfo_agent_deinit(void);

#endif /*_DEVICE_INFO*/