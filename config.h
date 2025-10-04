#ifndef CONFIG_H
#define CONFIG_H

#define MAX_WORKERS 100
#define MAX_TASKS 1000
#define SCHEDULER_PORT 8080
#define WORKER_PORT_BASE 9000
#define BUFFER_SIZE 1024
#define LOG_FILE "agendador.log"

#define HEARTBEAT_INTERVAL 30
#define WORKER_TIMEOUT 60

#endif