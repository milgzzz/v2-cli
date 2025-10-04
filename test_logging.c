#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../src/libtslog/tslog.h"

#define NUM_THREADS 5
#define LOGS_PER_THREAD 10

tslogger_t logger;

void* thread_function(void* arg) {
    int thread_id = *(int*)arg;
    char component[32];
    
    snprintf(component, sizeof(component), "Thread-%d", thread_id);
    
    for (int i = 0; i < LOGS_PER_THREAD; i++) {
        LOG_INFO(&logger, component, "Mensagem de log %d da thread", i);
        LOG_DEBUG(&logger, component, "Debug message %d", i);
        
        if (i % 3 == 0) {
            LOG_WARNING(&logger, component, "Aviso: contador múltiplo de 3");
        }
        
        if (i == LOGS_PER_THREAD - 1) {
            LOG_ERROR(&logger, component, "Última mensagem de erro");
        }
        
        usleep(100000); 
    }
    
    return NULL;
}

int main() {
    printf("Iniciando teste de logging com múltiplas threads...\n");
    
    if (tslog_init(&logger, "test_logging.log", LOG_DEBUG, 1) != 0) {
        fprintf(stderr, "Erro ao inicializar logger\n");
        return 1;
    }
    
    LOG_INFO(&logger, "Main", "Logger inicializado. Iniciando %d threads...", NUM_THREADS);
    
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            LOG_ERROR(&logger, "Main", "Erro ao criar thread %d", i);
            return 1;
        }
    }
    
    LOG_INFO(&logger, "Main", "Todas as threads criadas. Aguardando conclusão...");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    LOG_INFO(&logger, "Main", "Todas as threads finalizadas. Encerrando...");
    
    tslog_destroy(&logger);
    
    printf("Teste concluído. Verifique o arquivo test_logging.log\n");
    return 0;
}