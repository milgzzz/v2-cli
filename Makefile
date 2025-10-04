CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread -g -O2
INCLUDES = -Iinclude -Ilibtslog
LIBS = -pthread
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

TARGETS = $(BIN_DIR)/scheduler $(BIN_DIR)/worker $(BIN_DIR)/test_logging

.PHONY: all clean install test

all: $(TARGETS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

LIBTSLOG_OBJ = $(OBJ_DIR)/tslog.o

$(OBJ_DIR)/tslog.o: libtslog/tslog.c include/tslog.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

SCHEDULER_OBJ = $(OBJ_DIR)/scheduler.o $(OBJ_DIR)/network.o $(OBJ_DIR)/task_queue.o

$(BIN_DIR)/scheduler: $(SCHEDULER_OBJ) $(LIBTSLOG_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/scheduler.o: src/scheduler/scheduler.c include/scheduler.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

WORKER_OBJ = $(OBJ_DIR)/worker.o $(OBJ_DIR)/network.o

$(BIN_DIR)/worker: $(WORKER_OBJ) $(LIBTSLOG_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/worker.o: src/worker/worker.c include/worker.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR)/test_logging: tests/test_logging.c $(LIBTSLOG_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(OBJ_DIR)/network.o: src/common/network.c include/network.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/task_queue.o: src/common/task_queue.c include/task_queue.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(BIN_DIR)/test_logging
	$(BIN_DIR)/test_logging

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.log

install: all
	@echo "Instalando em /usr/local/bin/"
	@sudo cp $(BIN_DIR)/scheduler $(BIN_DIR)/worker /usr/local/bin/ || echo "Necessário sudo para instalação"

include Makefile.deps