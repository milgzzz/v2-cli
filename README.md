# 1. Primeiro preparar a estrutura
mkdir -p bin obj logs
make depend

# 2. Compilar tudo
make all

# 3. Testar individualmente
make test

# 4. Executar em terminais separados:
# Terminal 1 - Scheduler
./bin/scheduler --port 8080 --log-level INFO

# Terminal 2 - Worker 1
./bin/worker --scheduler-host localhost --scheduler-port 8080 --worker-id worker1

# Terminal 3 - Worker 2  
./bin/worker --scheduler-host localhost --scheduler-port 8080 --worker-id worker2

# Terminal 4 - Cliente de teste
./bin/test_client --scheduler-host localhost --scheduler-port 8080