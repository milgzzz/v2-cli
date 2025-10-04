#ifndef TSLOG_H
#define TSLOG_H

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "../common/types.h"

typedef struct {
    FILE *log_file;
    pthread_mutex_t lock;
    log_level_t min_level;
    int enable_console;
} tslogger_t;

int tslog_init(tslogger_t *logger, const char *filename, log_level_t min_level, int enable_console);
void tslog_destroy(tslogger_t *logger);

void tslog_write(tslogger_t *logger, log_level_t level, const char *component, const char *format, ...);

#define LOG_DEBUG(logger, component, ...) tslog_write(logger, LOG_DEBUG, component, __VA_ARGS__)
#define LOG_INFO(logger, component, ...) tslog_write(logger, LOG_INFO, component, __VA_ARGS__)
#define LOG_WARNING(logger, component, ...) tslog_write(logger, LOG_WARNING, component, __VA_ARGS__)
#define LOG_ERROR(logger, component, ...) tslog_write(logger, LOG_ERROR, component, __VA_ARGS__)
#define LOG_CRITICAL(logger, component, ...) tslog_write(logger, LOG_CRITICAL, component, __VA_ARGS__)

#endif