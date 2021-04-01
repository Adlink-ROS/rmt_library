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
    { 0, 0, 0, 0},
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

    // Parse argument
    int cmd_opt = 0;
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
    rmt_server_get_info(id_list, "cpu", dev_num);
    free(id_list);
    rmt_server_free_device_list(&dev_ptr);
    rmt_server_deinit();

    return 0;
}