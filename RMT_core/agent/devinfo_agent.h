#ifndef _DEVICEINFO_AGENT_
#define _DEVICEINFO_AGENT_

int devinfo_agent_config(char *interface, int id);
int devinfo_agent_update(struct dds_transport *transport);

#endif /*_DEVICEINFO_AGENT_*/