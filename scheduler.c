#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

int scheduler_init(scheduler_t *scheduler, int port) {
    if (tslog_init(&scheduler->logger, "scheduler.log", LOG_DEBUG, 1) != 0) {
        fprintf(stderr, "Erro ao inicializar logger do scheduler\n");
        return -1;
    }
    
    scheduler->server_sock = create_server_socket(port, &scheduler->logger);
    if (scheduler->server_sock < 0) {
        tslog_write(&scheduler->logger, LOG_CRITICAL, "SCHEDULER", "Falha ao criar socket do servidor");
        return -1;
    }
    
    scheduler->task_count = 0;
    scheduler->client_count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        scheduler->client_sockets[i] = -1;
    }
    
    tslog_write(&scheduler->logger, LOG_INFO, "SCHEDULER", "Scheduler inicializado na porta %d", port);
    return 0;
}

void handle_submit_task(scheduler_t *scheduler, int client_sock, message_t *msg) {
    if (scheduler->task_count >= MAX_TASKS) {
        tslog_write(&scheduler->logger, LOG_WARNING, "SCHEDULER", "Limite de tarefas atingido");
        
        message_t response;
        response.type = MSG_TASK_RESULT;
        response.size = snprintf(response.data, sizeof(response.data), "ERRO:Limite de tarefas excedido");
        send_message(client_sock, &response, &scheduler->logger);
        return;
    }
    
    // Simular processamento da tarefa
    task_t *task = &scheduler->tasks[scheduler->task_count];
    task->task_id = time(NULL) + scheduler->task_count;
    snprintf(task->task_type, sizeof(task->task_type), "CLI_TASK");
    strncpy(task->payload, msg->data, sizeof(task->payload));
    task->priority = 1;
    task->created_at = time(NULL);
    task->scheduled_for = time(NULL) + 5; // Executar em 5 segundos
    task->status = TASK_PENDING;
    
    tslog_write(&scheduler->logger, LOG_INFO, "SCHEDULER", 
                "Tarefa %lu submetida: %s", task->task_id, task->payload);
    
    scheduler->task_count++;
    
    // Enviar confirmação
    message_t response;
    response.type = MSG_TASK_RESULT;
    response.size = snprintf(response.data, sizeof(response.data), 
                           "SUCESSO:Tarefa %lu agendada", task->task_id);
    send_message(client_sock, &response, &scheduler->logger);
}

void handle_request_status(scheduler_t *scheduler, int client_sock, message_t *msg) {
    uint64_t task_id;
    sscanf(msg->data, "%lu", &task_id);
    
    char status_msg[256];
    int found = 0;
    
    for (int i = 0; i < scheduler->task_count; i++) {
        if (scheduler->tasks[i].task_id == task_id) {
            snprintf(status_msg, sizeof(status_msg), 
                    "Tarefa %lu: Status=%d, Payload=%s", 
                    task_id, scheduler->tasks[i].status, scheduler->tasks[i].payload);
            found = 1;
            break;
        }
    }
    
    if (!found) {
        snprintf(status_msg, sizeof(status_msg), "Tarefa %lu não encontrada", task_id);
    }
    
    message_t response;
    response.type = MSG_TASK_RESULT;
    response.size = snprintf(response.data, sizeof(response.data), "%s", status_msg);
    send_message(client_sock, &response, &scheduler->logger);
    
    tslog_write(&scheduler->logger, LOG_INFO, "SCHEDULER", 
                "Consulta de status: %s", status_msg);
}

void scheduler_run(scheduler_t *scheduler) {
    fd_set read_fds;
    int max_sd;
    
    tslog_write(&scheduler->logger, LOG_INFO, "SCHEDULER", "Iniciando loop principal");
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(scheduler->server_sock, &read_fds);
        max_sd = scheduler->server_sock;
        
        // Adicionar clientes ao conjunto
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (scheduler->client_sockets[i] > 0) {
                FD_SET(scheduler->client_sockets[i], &read_fds);
            }
            if (scheduler->client_sockets[i] > max_sd) {
                max_sd = scheduler->client_sockets[i];
            }
        }
        
        // Esperar por atividade
        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            tslog_write(&scheduler->logger, LOG_ERROR, "SCHEDULER", "Erro no select: %s", strerror(errno));
            continue;
        }
        
        // Nova conexão
        if (FD_ISSET(scheduler->server_sock, &read_fds)) {
            int new_socket = accept_client_connection(scheduler->server_sock, &scheduler->logger);
            if (new_socket >= 0) {
                // Adicionar novo cliente
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (scheduler->client_sockets[i] == -1) {
                        scheduler->client_sockets[i] = new_socket;
                        scheduler->client_count++;
                        tslog_write(&scheduler->logger, LOG_INFO, "SCHEDULER", 
                                   "Novo cliente aceito (total: %d)", scheduler->client_count);
                        break;
                    }
                }
            }
        }
        
        // Processar mensagens dos clientes
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = scheduler->client_sockets[i];
            if (sd > 0 && FD_ISSET(sd, &read_fds)) {
                message_t msg;
                if (receive_message(sd, &msg, &scheduler->logger) == 0) {
                    switch (msg.type) {
                        case MSG_SUBMIT_TASK:
                            handle_submit_task(scheduler, sd, &msg);
                            break;
                        case MSG_REQUEST_STATUS:
                            handle_request_status(scheduler, sd, &msg);
                            break;
                        default:
                            tslog_write(&scheduler->logger, LOG_WARNING, "SCHEDULER", 
                                       "Tipo de mensagem desconhecido: %d", msg.type);
                    }
                } else {
                    // Cliente desconectou
                    close(sd);
                    scheduler->client_sockets[i] = -1;
                    scheduler->client_count--;
                    tslog_write(&scheduler->logger, LOG_INFO, "SCHEDULER", 
                               "Cliente desconectado (restantes: %d)", scheduler->client_count);
                }
            }
        }
    }
}

void scheduler_cleanup(scheduler_t *scheduler) {
    close(scheduler->server_sock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (scheduler->client_sockets[i] > 0) {
            close(scheduler->client_sockets[i]);
        }
    }
    tslog_destroy(&scheduler->logger);
}