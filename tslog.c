#include "tslog.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

static const char* level_to_string(log_level_t level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        case LOG_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

static void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

int tslog_init(tslogger_t *logger, const char *filename, log_level_t min_level, int enable_console) {
    if (!logger) return -1;

    if (pthread_mutex_init(&logger->lock, NULL) != 0) {
        return -1;
    }

    if (filename) {
        logger->log_file = fopen(filename, "a");
        if (!logger->log_file) {
            pthread_mutex_destroy(&logger->lock);
            return -1;
        }
    } else {
        logger->log_file = NULL;
    }

    logger->min_level = min_level;
    logger->enable_console = enable_console;

    return 0;
}

void tslog_destroy(tslogger_t *logger) {
    if (!logger) return;

    pthread_mutex_lock(&logger->lock);
    
    if (logger->log_file) {
        fclose(logger->log_file);
        logger->log_file = NULL;
    }
    
    pthread_mutex_unlock(&logger->lock);
    pthread_mutex_destroy(&logger->lock);
}

void tslog_write(tslogger_t *logger, log_level_t level, const char *component, const char *format, ...) {
    if (!logger || level < logger->min_level) return;

    va_list args;
    char timestamp[20];
    char message[512];
    char log_line[1024];

    get_timestamp(timestamp, sizeof(timestamp));

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    snprintf(log_line, sizeof(log_line), "[%s] [%s] [%s] [thread:%lu] %s\n",
             timestamp, level_to_string(level), component, 
             (unsigned long)pthread_self(), message);

    pthread_mutex_lock(&logger->lock);

    if (logger->enable_console) {
        printf("%s", log_line);
        fflush(stdout);
    }

    if (logger->log_file) {
        fprintf(logger->log_file, "%s", log_line);
        fflush(logger->log_file);
    }

    pthread_mutex_unlock(&logger->lock);
}