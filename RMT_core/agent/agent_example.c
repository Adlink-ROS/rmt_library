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
    int ret = 0;
    int cpu_usage;
    char column[10];
    unsigned int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    unsigned int total_jiffies[2], work_jiffies[2];
    FILE *fp;

    if (!payload) return -1;

    for (int i = 0; i < 2; i++) {
        fp = fopen("/proc/stat", "r");
        if (!fp) {
            ret = -1;
            goto exit;
        }
        ret = fscanf(fp, "%s %u %u %u %u %u %u %u %u %u %u", column, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
        if (ret < 0) {
            ret = -1;
            fclose(fp);
            goto exit;
        }
        total_jiffies[i] = user + nice + system + idle + iowait + irq + softirq;
        work_jiffies[i] = user + nice + system;
        fclose(fp);
        if (i == 0) usleep(500000); // sleep 500ms
    }

    cpu_usage = (work_jiffies[1] - work_jiffies[0]) * 100 / (total_jiffies[1] - total_jiffies[0]);

    printf("cpu usage: %d\n", cpu_usage);
    sprintf(payload, "%d", cpu_usage);

exit:
    return ret;
}

int get_ram(char *payload)
{
    int ret = 0;
    char column[20], dummy[20];
    unsigned int mem_value[5];
    unsigned int total_mem, free_mem, buffer_mem, cached_mem;
    FILE *fp;
    int ram_usage;

    if (!payload) return -1;

    fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        ret = -1;
        goto exit;
    }
    for (int i = 0; i < 5; i++) {
        ret = fscanf(fp, "%s %u %s", column, &mem_value[i], dummy);
        if (ret < 0) {
            ret = -1;
            fclose(fp);
            goto exit;
        }
    }
    total_mem = mem_value[0];
    free_mem = mem_value[1];
    buffer_mem = mem_value[3];
    cached_mem = mem_value[4];
    fclose(fp);

    printf("Total memory: %d\n", total_mem);
    printf("Available memory: %d\n", free_mem + buffer_mem + cached_mem);
    printf("Used memory: %d\n", total_mem - free_mem - buffer_mem - cached_mem);

    ram_usage = (total_mem - free_mem - buffer_mem - cached_mem) * 100 / total_mem;

    printf("RAM usage: %d\n", ram_usage);
    sprintf(payload, "%d", ram_usage);

exit:
    return ret;
}

int get_hostname(char *payload)
{
    char hostname[1024];

    if (!payload) return -1;

    gethostname(hostname, sizeof(hostname));
    printf("hostname: %s\n", hostname);
    sprintf(payload, "%s", hostname);

    return 0;
}

// RMT_TODO: show correct data
int get_wifi(char *payload)
{
    char *ssid = "myssid";

    printf("ssid: %s\n", ssid);
    if (payload) {
        sprintf(payload, "%s", ssid);
    }
    return 0;
}

static datainfo_func func_maps[] = {
    {"cpu",      get_cpu,      NULL},
    {"ram",      get_ram,      NULL},
    {"hostname", get_hostname, NULL},
    {"wifi",     get_wifi,     NULL},
    {0,          0,            0   },
};

char *short_options = "i:n:h";
struct option long_options[] = {
    {"id",   required_argument, NULL, 'i'},
    {"net",  required_argument, NULL, 'n'},
    {"help", no_argument,       NULL, 'h'},
    { 0,     0,                 0,    0  },
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
    int cmd_opt = 0;

    // Parse argument
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
