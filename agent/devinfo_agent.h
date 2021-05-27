#ifndef _DEVICEINFO_AGENT_
#define _DEVICEINFO_AGENT_

#include <stdint.h>

int devinfo_agent_config(char *interface, uint64_t id);
int devinfo_agent_update(struct dds_transport *transport);
unsigned long devinfo_get_id(void);

#endif /*_DEVICEINFO_AGENT_*/
