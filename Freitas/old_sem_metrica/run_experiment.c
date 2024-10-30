#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"

void run_experiment(int cpu_bound_procs, int io_bound_procs) {
    for (int i = 0; i < cpu_bound_procs; i++) {
        if (fork() == 0) {
            exec("cpu_bound", 0);
            exit();
        }
    }

    for (int i = 0; i < io_bound_procs; i++) {
        if (fork() == 0) {
            exec("io_bound", 0);
            exit();
        }
    }

    // Esperar processos filhos finalizarem e coletar métricas (latência de IO, vazão, etc.)
    for (int i = 0; i < cpu_bound_procs + io_bound_procs; i++) {
        wait();
    }

    // Calcular métricas e exibir o desempenho geral do sistema
    printf(1, "Rodada concluída\n");
}

int main(int argc, char *argv[]) {
    int cpu_bound_procs = atoi(argv[1]);
    int io_bound_procs = 20 - cpu_bound_procs;

    for (int round = 0; round < 30; round++) {
        printf(1, "Rodada %d\n", round + 1);
        run_experiment(cpu_bound_procs, io_bound_procs);
    }
    exit();
}
