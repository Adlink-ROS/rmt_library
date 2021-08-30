#include <string.h>
#include "logger.h"
#include "rmt_config.h"

FILE *g_fp = NULL;
int g_log_disable = 0;

void log_init(void)
{
    /* Disabe log or not */
    if (strcmp(g_rmt_cfg.logfile, "none") == 0) {
        g_log_disable = 1;
        return;
    }

    if (strcmp(g_rmt_cfg.logfile, "stderr") == 0) {
        g_fp = stderr;
    } else {
        g_fp = fopen(g_rmt_cfg.logfile, "a");
        if (!g_fp) {
            g_fp = stderr;
            RMT_ERROR("Unable to open the log file.\n");
        }
    }
}

void log_deinit(void)
{
    if (strcmp(g_rmt_cfg.logfile, "none") == 0) {
        g_log_disable = 0;
        return;
    }

    if (g_fp) {
        if (g_fp != stderr) {
            fclose(g_fp);
        }
        g_fp = NULL;
    }
}
