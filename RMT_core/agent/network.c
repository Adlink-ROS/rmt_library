#include <string.h>

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