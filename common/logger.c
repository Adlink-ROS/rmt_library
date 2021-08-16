#include "logger.h"

#define LOG_NAME "/tmp/rmt.log"

FILE *g_fp; 

void log_init(void) {
    // RMT_TODO: Use config to control the log
    if (0) {
        g_fp = fopen(LOG_NAME, "a");
        if (!g_fp) {
            g_fp = stderr;
            RMT_ERROR("Unable to open the log file.\n");
        }
    } else {
        g_fp = stderr;
    }
}

void log_deinit(void) {
    if (g_fp && g_fp != stderr) {
        fclose(g_fp);
    }
}