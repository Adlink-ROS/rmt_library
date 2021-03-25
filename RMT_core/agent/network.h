#ifndef _NETWORK_H_
#define _NETWORK_H_

int select_interface(char *interface);
int get_ip(char *interface, char *ip, int ip_len);
int get_mac(char *interface, char *mac, int mac_len);
uint64_t get_id_from_mac(char *interface);

#endif /*_NETWORK_H_*/