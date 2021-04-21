#ifndef _NETWORK_H_
#define _NETWORK_H_

int net_select_interface(char *interface);
int net_get_ip(char *interface, char *ip, int ip_len);
int net_get_mac(char *interface, char *mac, int mac_len);
uint64_t net_get_id_from_mac(char *interface);

#endif /*_NETWORK_H_*/
