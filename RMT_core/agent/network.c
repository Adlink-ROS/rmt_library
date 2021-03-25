#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "network.h"

int select_interface(char *interface)
{
    int ret = -1;
    struct if_nameindex *if_nidxs, *intf;
    struct ifreq ifr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&ifr, sizeof(ifr));

    if_nidxs = if_nameindex();
    if (if_nidxs == NULL) {
        ret = -1;
        goto exit;
    }
    for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
        strcpy(ifr.ifr_name, intf->if_name);
        ioctl(sockfd, SIOCGIFFLAGS, &ifr);
        // Make sure the network interface is up and running. Also exclude loopback.
        if ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING) && !(ifr.ifr_flags & IFF_LOOPBACK)) {
            strcpy(interface, intf->if_name);
            ret = 0;
        }
    }
    if_freenameindex(if_nidxs);
exit:
    close(sockfd);
    return ret;
}

int get_ip(char *interface, char *ip, int ip_len)
{
    int fd;
    struct ifreq ifr;
   
    fd = socket(AF_INET, SOCK_DGRAM, 0);
   
    ifr.ifr_addr.sa_family = AF_INET; // get IPv4 address
    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1); // get IP from certain interface
    ioctl(fd, SIOCGIFADDR, &ifr); // get IP
   
    close(fd);

    strncpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ip_len-1);
    ip[ip_len-1] = 0;
   
    return 0;
}

int get_mac(char *interface, char *mac, int mac_len)
{
    struct ifreq ifr;
    int fd;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1); // get MAC from certain interface
    ioctl(fd, SIOCGIFHWADDR, &ifr);

    close(fd);

    snprintf(mac, mac_len, "%02x:%02x:%02x:%02x:%02x:%02x", 
            (unsigned char) ifr.ifr_hwaddr.sa_data[0],
            (unsigned char) ifr.ifr_hwaddr.sa_data[1],
            (unsigned char) ifr.ifr_hwaddr.sa_data[2],
            (unsigned char) ifr.ifr_hwaddr.sa_data[3],
            (unsigned char) ifr.ifr_hwaddr.sa_data[4],
            (unsigned char) ifr.ifr_hwaddr.sa_data[5]);

    return 0;
}

uint64_t get_id_from_mac(char *interface)
{
    uint64_t id = 0;
    struct ifreq ifr;
    int fd;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1); // get MAC from certain interface
    ioctl(fd, SIOCGIFHWADDR, &ifr);

    close(fd);

    for (int i = 0; i < 6; i++) {
        id <<= 8;
        id += ifr.ifr_hwaddr.sa_data[i];
    }

    return id;
}