#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"

// Funções para medir o tempo
uint get_time() {
    return uptime();
}

// Variável global para definir o modo de execução
int otimizador_escalonado = 0;

// Função para rodar os experimentos e coletar métricas
// Função para rodar os experimentos e coletar métricas
void run_experiment(int cpu_bound_procs, int io_bound_procs) {
    uint start_time, end_time;
    int total_io_latency = 0, total_throughput = 0;
    int completed_processes = 0;

    // Iniciar processos CPU-bound
    for (int i = 0; i < cpu_bound_procs; i++) {
        int pid = fork();
        if (pid == 0) {
            if (exec("cpu_bound", 0) < 0) {
                printf(1, "Erro ao executar cpu_bound\n");
                exit();
            }
        } else if (pid < 0) {
            printf(1, "Erro ao criar processo CPU-bound\n");
        }
    }

    // Iniciar processos IO-bound
    for (int i = 0; i < io_bound_procs; i++) {
        int pid = fork();
        if (pid == 0) {
            if (exec("io_bound", 0) < 0) {
                printf(1, "Erro ao executar io_bound\n");
                exit();
            }
        } else if (pid < 0) {
            printf(1, "Erro ao criar processo IO-bound\n");
        }
    }

    // Medir métricas para cada processo filho
    start_time = get_time();
    for (int i = 0; i < cpu_bound_procs + io_bound_procs; i++) {
        if (wait() >= 0) {  // Espera cada processo filho finalizar
            end_time = get_time();

            // Coleta de métricas
            int io_latency = end_time - start_time;
            total_io_latency += io_latency;
            total_throughput += (1000 / io_latency); // processos/segundo
            completed_processes++;
        }
    }

    // Calcular métricas finais
    if (completed_processes > 0) {
        int avg_io_latency = total_io_latency / completed_processes;
        int avg_throughput = total_throughput / completed_processes;
        printf(1, "Rodada concluída | Latência de IO: %d | Vazão: %d | Otimizador: %d\n", avg_io_latency, avg_throughput, otimizador_escalonado);
    }
}


int main(int argc, char *argv[]) {
    int cpu_bound_procs = atoi(argv[1]);
    int io_bound_procs = 20 - cpu_bound_procs;

    // Definir o modo de execução: XV6 Original ou Otimizado
    if (argc > 2) {
        otimizador_escalonado = atoi(argv[2]);
    }

    for (int round = 0; round < 30; round++) {
        printf(1, "Rodada %d\n", round + 1);
        run_experiment(cpu_bound_procs, io_bound_procs);
    }

    exit();
}
/*  */