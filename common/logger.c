#include <string.h>
#include "logger.h"
#include "config.h"

FILE *g_fp = NULL;

void log_init(void)
{
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
    if (g_fp) {
        if (g_fp != stderr) {
            fclose(g_fp);
        }
        g_fp = NULL;
    }
}
