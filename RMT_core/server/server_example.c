#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "rmt_server.h"

static char *my_interface = NULL;
static char interface[50];

char *short_options = "n:h";
struct option long_options[] = {
    {"net",  required_argument, NULL, 'n'},
    {"help", no_argument,       NULL, 'h'},
    { 0,     0,                 0,    0  },
};

void print_help(void)
{
    printf("Usage: ./server_example [options]\n");
    printf("* --help: Showing this messages.\n");
    printf("* --net [interface]: Decide which interface agent uses.\n");
}

int main(int argc, char *argv[])
{
    int dev_num;
    device_info *dev_ptr;
    unsigned long *id_list;
    int cmd_opt = 0;

    // Parse argument
    while ((cmd_opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (cmd_opt) {
            case 'n':
                strcpy(interface, optarg);
                my_interface = interface;
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
            default:
                printf("Not supported option\n");
                return 1;
        }
    }

    printf("RMT Library version is %s\n", rmt_server_version());
    rmt_server_config(my_interface);
    rmt_server_init();
    // get device list
    dev_ptr = rmt_server_create_device_list(&dev_num);
    for (int i = 0; i < dev_num; i++) {
        printf("Device %d\n", i);
        printf("ID: %lu\n", dev_ptr[i].deviceID);
        printf("Model: %s\n", dev_ptr[i].model);
        printf("Host: %s\n", dev_ptr[i].host);
        printf("IP: %s\n", dev_ptr[i].ip);
        printf("MAC: %s\n", dev_ptr[i].mac);
        printf("RMT version: %s\n", dev_ptr[i].rmt_version);
        fflush (stdout);
    }
    // assign id to id_list
    id_list = (unsigned long *) malloc(sizeof(unsigned long) * dev_num);
    for (int i = 0; i < dev_num; i++) {
        id_list[i] = dev_ptr[i].deviceID;
    }
    // get server_info
    int info_list_num;
    data_info *info_list = rmt_server_get_info(id_list, dev_num, "cpu;ram;hostname;wifi;", &info_list_num);
    printf("Try to get info from %d device\n", info_list_num);
    for (int i = 0; i < info_list_num; i++) {
        printf("ID: %ld\n", info_list[i].deviceID);
        printf("return list: %s\n", info_list[i].value_list);
    }
    rmt_server_free_info(info_list, info_list_num);
    free(id_list);
    // set data_info
    data_info set_info;
    printf("Try to set info to id 6166\n");
    set_info.deviceID = 6166;
    set_info.value_list = "hostname:ros-ROScube-I;locate:on;";
    info_list = rmt_server_set_info(&set_info, 1, &info_list_num);
    for (int i = 0; i < info_list_num; i++) {
        printf("ID: %ld\n", info_list[i].deviceID);
        printf("return list: %s\n", info_list[i].value_list);
    }
    rmt_server_free_info(info_list, info_list_num);
    // free resource
    rmt_server_free_device_list(dev_ptr);
    rmt_server_deinit();

    return 0;
}
