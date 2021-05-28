#ifndef _DEVICEINFO_AGENT_
#define _DEVICEINFO_AGENT_

#include "rmt_agent.h"

int devinfo_agent_init(devinfo_func serach_func);
void devinfo_agent_deinit(void);
int devinfo_agent_update(struct dds_transport *transport);
unsigned long devinfo_get_id(void);

#endif /*_DEVICEINFO_AGENT_*/
