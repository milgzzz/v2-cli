#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <time.h>

typedef enum {
    TASK_PENDING,
    TASK_RUNNING,
    TASK_COMPLETED,
    TASK_FAILED
} task_status_t;

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} log_level_t;

typedef struct {
    uint64_t task_id;
    char task_type[32];
    char payload[256];
    int priority;
    time_t created_at;
    time_t scheduled_for;
    task_status_t status;
    char assigned_worker[64];
} task_t;

typedef struct {
    char worker_id[64];
    char host[64];
    int port;
    int load;
    time_t last_heartbeat;
    int is_active;
} worker_info_t;

#endif