#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"

int total_processes_completed = 0;

// Função principal do experimento
void run_experiment(int cpu_bound_procs, int io_bound_procs) {
    int start_time = uptime();

    // Inicia processos CPU-bound
    for (int i = 0; i < cpu_bound_procs; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Erro ao criar processo cpu_bound\n");
            exit();
        }
        if (pid == 0) {
            // Processo filho
            char *args[] = { "cpu_bound", 0 };
            exec("cpu_bound", args);
            printf(1, "Erro ao executar cpu_bound\n");
            exit();
        }
    }

    for (int i = 0; i < io_bound_procs; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Erro ao criar processo io_bound\n");
            exit();
        }
        if (pid == 0) {
            // Processo filho
            char *args[] = { "io_bound", 0 };
            exec("io_bound", args);
            printf(1, "Erro ao executar io_bound\n");
            exit();
        }
        sleep(10); // Pausa de 10 ticks entre a criação de processos
    }

    // Espera pelos processos filhos
    for (int i = 0; i < cpu_bound_procs + io_bound_procs; i++) {
        wait();
        total_processes_completed++;
    }

    int end_time = uptime();
    int execution_time = end_time - start_time;

    // Exibe o tempo de execução da rodada
    printf(1, "Tempo de execução da rodada: %d ticks\n", execution_time);
    printf(1, "-----------------------------------------\n");
}

int main(int argc, char *argv[]) {
    int cpu_bound_procs = 10; // Valor padrão

    // Verifica argumentos de linha de comando
    if (argc > 1) {
        cpu_bound_procs = atoi(argv[1]);
        if (cpu_bound_procs < 6 || cpu_bound_procs > 14) {
            printf(1, "Número de processos CPU-bound deve ser entre 6 e 14\n");
            exit();
        }
    }

    int io_bound_procs = 20 - cpu_bound_procs;

    // Executa 30 rodadas do experimento
    for (int round = 0; round < 30; round++) {
        printf(1, "Rodada %d", round + 1);
        run_experiment(cpu_bound_procs, io_bound_procs);
    }
    printf(1, "Experimento concluído com %d processos completados\n", total_processes_completed);
    exit();
}