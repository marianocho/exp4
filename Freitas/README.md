### Resumo de Configuração e Execução do XV6 com Docker e QEMU

#### 1. Como Dar Build e Run no Docker

Para compilar e rodar o XV6 no Docker:

1. **Compilar a Imagem Docker**:
   - No terminal, vá para a pasta `xv6-public` onde está o Dockerfile e execute:
     ```bash
     docker build -t xv6 .
     ```

2. **Executar o Container Docker**:
   - Para iniciar o container com o QEMU executando o XV6, rode o comando:
     ```bash
     docker run --rm -it xv6
     ```

#### 2. Como Setar o QEMU Usando o Terminal do Docker

No terminal Docker, o QEMU iniciará automaticamente. Para interagir com o QEMU:

1. **Reiniciar o QEMU Manualmente (Opcional)**:
   - Se você realizar alterações no código do XV6, saia do QEMU e execute:
     ```bash
     make clean
     make
     make qemu-nox
     ```
   - Isso recompilará o sistema com as novas alterações e reiniciará o QEMU.

#### 3. Como Usar o Comando `run_experiment`

Após configurar e compilar o `run_experiment`, execute-o no ambiente do QEMU:

1. **Executar o Experimento**:
   - No terminal do QEMU, digite:
     ```bash
     run_experiment 10
     ```
   - O argumento `10` representa o número de processos **CPU-bound**. O programa calcula automaticamente o número de processos **IO-bound** como `20 - CPU-bound`.

2. **Exibir Métricas**:
   - O comando `run_experiment` executa 30 rodadas, exibindo as seguintes métricas ao final de cada rodada:
     - Latência de I/O
     - Throughput (vazão)
     - Justiça entre processos
     - Eficiência do sistema de arquivos
     - Overhead de gerenciamento de memória
     - Desempenho geral do sistema

---

### Extra: Documentação dos Códigos Implementados

#### 1. `run_experiment.c`

- **Descrição**: Controla a execução de experimentos com processos CPU-bound e IO-bound, coletando métricas de desempenho.
- **Funções**:
  - `run_experiment(int cpu_bound_procs, int io_bound_procs)`: Inicia os processos, aguarda a finalização e coleta métricas.
  - `calculate_io_latency()`: Mede a latência de operações de I/O.
  - `calculate_throughput(int execution_time, int total_procs)`: Calcula a vazão (throughput) dos processos.
  - `calculate_fairness(int total_procs)`: Mede a justiça entre processos.
  - `calculate_fs_efficiency()`: Avalia a eficiência do sistema de arquivos.
  - `calculate_memory_overhead()`: Calcula o overhead de gerenciamento de memória.
  - `calculate_system_performance(...)`: Calcula o desempenho geral do sistema usando métricas normalizadas.

#### 2. `cpu_bound.c`

- **Descrição**: Implementa um processo CPU-bound simulando o cálculo do menor caminho em grafos.
- **Funções**:
  - `initialize_random_graph(int vertices, int edges)`: Cria um grafo aleatório com pesos.
  - `min_distance(int vertices)`: Encontra o vértice com a menor distância no grafo.
  - `dijkstra(int src, int vertices)`: Calcula o menor caminho usando o algoritmo de Dijkstra.
  - `generate_and_solve_graph()`: Gera grafos e resolve o menor caminho.

#### 3. `io_bound.c`

- **Descrição**: Implementa um processo IO-bound que escreve, altera e deleta dados em um arquivo.
- **Funções**:
  - `generate_random_line(char *line)`: Gera uma linha de texto aleatório.
  - `perform_io_operations()`: Executa operações de escrita, leitura e permutação em um arquivo para simular carga de I/O.