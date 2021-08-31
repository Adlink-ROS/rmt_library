#ifndef _LOGGER_
#define _LOGGER_

#include <stdio.h>
#include <time.h>

#define RMT_LOG_ENABLE   1
#define RMT_WARN_ENABLE  1
#define RMT_ERROR_ENABLE 1

#if RMT_LOG_ENABLE
 # define RMT_LOG(...)             \
          do {                     \
              if (g_log_disable)   \
                  break;           \
              FILE *fp = stderr;   \
              if (g_fp != NULL)    \
                  fp = g_fp;       \
              time_t now_time;     \
              time(&now_time);     \
              struct tm *now_tm_local = localtime(&now_time);                     \
              fprintf(fp, "%02d/%02d %02d:%02d:%02d  ", (1+now_tm_local->tm_mon), \
                                                        (now_tm_local->tm_mday),  \
                                                        now_tm_local->tm_hour,    \
                                                        now_tm_local->tm_min,     \
                                                        now_tm_local->tm_sec);    \
              fprintf(fp, "RMT_LOG:");  \
              fprintf(fp, __VA_ARGS__); \
              fflush ((FILE *) fp);     \
          } while(0);
#else
 # define RMT_LOG
#endif /*RMT_LOG_ENABLE*/

#if RMT_WARN_ENABLE
 # define RMT_WARN(...)            \
          do {                     \
              if (g_log_disable)   \
                  break;           \
              FILE *fp = stderr;   \
              if (g_fp != NULL)    \
                  fp = g_fp;       \
              time_t now_time;     \
              time(&now_time);     \
              struct tm *now_tm_local = localtime(&now_time);                     \
              fprintf(fp, "%02d/%02d %02d:%02d:%02d  ", (1+now_tm_local->tm_mon), \
                                                        (now_tm_local->tm_mday),  \
                                                        now_tm_local->tm_hour,    \
                                                        now_tm_local->tm_min,     \
                                                        now_tm_local->tm_sec);    \
              fprintf(fp, "RMT_WARN:"); \
              fprintf(fp, __VA_ARGS__); \
              fflush ((FILE *) fp);     \
          } while(0);
#else
 # define RMT_WARN
#endif /*RMT_WARN_ENABLE*/

#if RMT_ERROR_ENABLE
 # define RMT_ERROR(...)           \
          do {                     \
              if (g_log_disable)   \
                  break;           \
              FILE *fp = stderr;   \
              if (g_fp != NULL)    \
                  fp = g_fp;       \
              time_t now_time;     \
              time(&now_time);     \
              struct tm *now_tm_local = localtime(&now_time);                     \
              fprintf(fp, "%02d/%02d %02d:%02d:%02d  ", (1+now_tm_local->tm_mon), \
                                                        (now_tm_local->tm_mday),  \
                                                        now_tm_local->tm_hour,    \
                                                        now_tm_local->tm_min,     \
                                                        now_tm_local->tm_sec);    \
              fprintf(fp, "RMT_ERROR:"); \
              fprintf(fp, __VA_ARGS__);  \
              fflush ((FILE *) fp);     \
          } while(0);
#else
 # define RMT_ERROR
#endif /*RMT_ERROR_ENABLE*/

extern FILE *g_fp;
extern int g_log_disable;

void log_init(void);
void log_deinit(void);

#endif /*_LOGGER_*/
