#ifndef _NETWORK_H_
#define _NETWORK_H_

int select_interface(char *interface);
int get_ip(char *interface, char *ip, int ip_len);

#endif /*_NETWORK_H_*/