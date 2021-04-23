#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // sleep
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include "rmt_agent.h"
#include "mraa/led.h"

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

int set_hostname(char *payload)
{
    if (!payload) return -1;

    printf("hostname to be set: %s\n", payload);
    if (sethostname(payload, strlen(payload)) != 0) {
        return -1;
    }

    return 0;
}

int get_wifi(char *payload)
{
    int ret = 0;
    char buffer[512];
    FILE *fp;
    char interface[24];
    int rssi;
    int interface_num = 0;

    if (!payload) return -1;

    fp = fopen("/proc/net/wireless", "r");
    if (!fp) {
        ret = -1;
        goto exit;
    }

    // skip the first 2 lines
    for (int i = 0; i < 2; i++) {
        if (!fgets(buffer, sizeof(buffer), fp)) {
            ret = -1;
            goto exit;
        }
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        // get interface and RSSI
        sscanf(buffer, "%[^:]: %*s %*d. %d. %*d %*d %*d %*d %*d %*d %*d\n",
               interface, &rssi);
        // get SSID
        int sock_fd;
        struct iwreq wreq;
        char ssid[IW_ESSID_MAX_SIZE + 1];

        memset(&wreq, 0, sizeof(struct iwreq));
        strcpy(wreq.ifr_name, interface);
        if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            ret = -1;
            goto exit;
        }
        wreq.u.essid.pointer = ssid;
        wreq.u.essid.length = IW_ESSID_MAX_SIZE;
        if (ioctl(sock_fd, SIOCGIWESSID, &wreq)) {
            ret = -1;
            goto exit;
        }
        close(sock_fd);

        printf("%s: ssid=%s rssi=%d\n", interface, ssid, rssi);
        if (interface_num != 0) {
            sprintf(payload, ",");
        }
        sprintf(payload, "%s %s %d", interface, ssid, rssi);
        interface_num++;
    }
    if (interface_num == 0) {
        sprintf(payload, "none");
    }
    fclose(fp);

exit:
    return ret;
}

#define LED_NUM 0
static int locate_on = 0;
int set_locate(char *payload)
{
    if (!payload) return -1;

    if (strcmp(payload, "on") == 0) {
        locate_on = 1;
    } else {
        locate_on = 0;
    }

    printf("set LED: %d\n", locate_on);

    return 0;
}

/*
 * status=0: dark
 * status=1: bright
 */
void set_led_status(int status)
{
    mraa_led_context led;

    led = mraa_led_init(LED_NUM);
    if (led == NULL) {
        printf("Unable to init LED\n");
        goto exit;
    }

    mraa_result_t result = mraa_led_set_brightness(led, status);
    if (result != MRAA_SUCCESS) {
        printf("Unable to set LED\n");
        goto exit;
    }

exit:
    if (led)
        mraa_led_close(led);
}

void locate_daemon(void)
{
    static int init = 0;
    static int led_status;
    static time_t start_time;

    if (!init) {
        init = 1;
        led_status = 0;
        start_time = time(NULL);
    }
    if (locate_on && (start_time != time(NULL))) {
        led_status = !led_status;
        set_led_status(led_status);
        start_time = time(NULL);
    }
    // If we do not locate the device, we need to make the led_status back to 0.
    // RMT_TODO: We need to close LED while user presses ctrl+C.
    if (!locate_on && (led_status != 0)) {
        led_status = 0;
        set_led_status(led_status);
    }
}

static datainfo_func func_maps[] = {
    {"cpu",      get_cpu,      NULL         },
    {"ram",      get_ram,      NULL         },
    {"hostname", get_hostname, set_hostname },
    {"wifi",     get_wifi,     NULL         },
    {"locate",   NULL,         set_locate   },
    {0,          0,            0            },
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

// RMT_TODO: Use C++ instead of C.
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
    mraa_init();
    while (1) {
        rmt_agent_running();
        locate_daemon();
        usleep(10000); // sleep 10ms
    }
    mraa_deinit();
    rmt_agent_deinit();
    return 0;
}
