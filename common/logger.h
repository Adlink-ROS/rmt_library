#ifndef _LOGGER_
#define _LOGGER_

#include <stdio.h>

#define RMT_LOG_ENABLE   1
#define RMT_WARN_ENABLE  1
#define RMT_ERROR_ENABLE 1

#if RMT_LOG_ENABLE
 # define RMT_LOG(...)             \
          do {                     \
              fprintf(g_fp, "RMT_LOG:");  \
              fprintf(g_fp, __VA_ARGS__); \
          } while(0);
#else
 # define RMT_LOG
#endif /*RMT_LOG_ENABLE*/

#if RMT_WARN_ENABLE
 # define RMT_WARN(...)            \
          do {                     \
              fprintf(g_fp, "RMT_WARN:"); \
              fprintf(g_fp, __VA_ARGS__); \
          } while(0);
#else
 # define RMT_WARN
#endif /*RMT_WARN_ENABLE*/

#if RMT_ERROR_ENABLE
 # define RMT_ERROR(...)           \
          do {                     \
              fprintf(g_fp, "RMT_ERROR:"); \
              fprintf(g_fp, __VA_ARGS__);  \
          } while(0);
#else
 # define RMT_ERROR
#endif /*RMT_ERROR_ENABLE*/

extern FILE *g_fp; 

void log_init(void);
void log_deinit(void);

#endif /*_LOGGER_*/
