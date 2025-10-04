# Arquitetura do Agendador Distribuído

## Componentes Principais

### 1. Scheduler (Agendador)
- **Função**: Coordenação central do sistema
- **Responsabilidades**:
  - Receber novas tarefas
  - Distribuir tarefas para workers
  - Monitorar status dos workers
  - Balanceamento de carga
  - Persistência de estado

### 2. Worker (Trabalhador)
- **Função**: Execução de tarefas
- **Responsabilidades**:
  - Registrar-se no scheduler
  - Executar tarefas atribuídas
  - Reportar status e resultados
  - Manter heartbeat

### 3. Biblioteca libtslog
- **Função**: Logging thread-safe
- **Características**:
  - API simples e clara
  - Suporte a múltiplos níveis de log
  - Output para console e arquivo
  - Timestamp e identificação de thread

## Diagrama de Comunicação
