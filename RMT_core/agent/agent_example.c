#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // sleep
#include <getopt.h>
#include <string.h>
#include "rmt_agent.h"

static unsigned long myid = 0;
static char *my_interface = NULL;
static char interface[50];

char *short_options = "i:n:";
struct option long_options[] = {
    {"id",  required_argument, NULL, 'i'},
    {"net", required_argument, NULL, 'n'},
    { 0, 0, 0, 0},
};

int main(int argc, char *argv[])
{
    // Parse argument
    int cmd_opt = 0;
    while ((cmd_opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (cmd_opt) {
            case 'i':
                myid = atoi(optarg);
                break;
            case 'n':
                strcpy(interface, optarg);
                my_interface = interface;
                break;
            case '?':
            default:
                printf("Not supported option\n");
                return 1;
        }
    }

    printf("This is RMT Agent. id=%lu and network interface=%s\n", myid, my_interface);
    rmt_agent_init(my_interface, myid);
    while (1) {
        rmt_agent_running();
        sleep(1);
    }
    rmt_agent_deinit();
    return 0;
}