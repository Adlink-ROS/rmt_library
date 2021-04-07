#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // sleep
#include <getopt.h>
#include <string.h>
#include "rmt_agent.h"

static unsigned long myid = 0;
static char *my_interface = NULL;
static char interface[50];

int get_cpu(char *payload)
{
    int cpu_usage = 20;
    printf("cpu usage: %d\n", cpu_usage);
    if (payload)
        sprintf(payload, "%d", cpu_usage);
    return 0;
}

int get_ram(char *payload)
{
    int ram_usage = 30;
    printf("RAM usage: %d\n", ram_usage);
    if (payload)
        sprintf(payload, "%d", ram_usage);
    return 0;
}

static datainfo_func func_maps[] = {
    {"cpu", get_cpu, NULL},
    {"ram", get_ram, NULL},
    {0, 0, 0},
};

char *short_options = "i:n:h";
struct option long_options[] = {
    {"id",   required_argument, NULL, 'i'},
    {"net",  required_argument, NULL, 'n'},
    {"help", no_argument,       NULL, 'h'},
    { 0, 0, 0, 0},
};

void print_help(void)
{
    printf("Usage: ./agent_example [options]\n");
    printf("* --help: Showing this messages.\n");
    printf("* --id [myID]: Use myID as the ID.\n");
    printf("* --net [interface]: Decide which interface agent uses.\n");
}

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
            case 'h':
                print_help();
                return 0;
            case '?':
            default:
                printf("Not supported option\n");
                print_help();
                return 1;
        }
    }

    printf("This is RMT Agent. id=%lu and network interface=%s\n", myid, my_interface);
    rmt_agent_config(my_interface, myid);
    rmt_agent_init(func_maps);
    while (1) {
        rmt_agent_running();
        usleep(10000); // sleep 10ms
    }
    rmt_agent_deinit();
    return 0;
}