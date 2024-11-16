// run_experiments.c
#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"
#include "utils.h"

#define MAX_PROCS 20  // Maximum number of processes
#define SCALE_FACTOR 1000000

unsigned int min_throughput = 0;
unsigned int max_throughput = 0;
unsigned int avg_throughput = 0;
int total_processes_completed = 0;

// Main function to run the experiment
void run_experiment(int cpu_bound_procs, int io_bound_procs, unsigned int *J_cpu_scaled, unsigned int *E_fs_scaled, unsigned int *M_over_scaled, unsigned int *norm_throughput) {
    int start_time = uptime();

    // Resetar métricas antes de cada rodada
    min_throughput = 0;
    max_throughput = 0;
    avg_throughput = 0;
    total_processes_completed = 0;

    int pids[MAX_PROCS];
    int pid_index = 0;

    int start_times[MAX_PROCS];
    int end_times[MAX_PROCS];
    int execution_times[MAX_PROCS];

    // Inicialize pid_index antes do loop
    pid_index = 0;

    // Loop para criar processos CPU-bound e IO-bound
    for (int i = 0; i < cpu_bound_procs || i < io_bound_procs; i++) {
        if (i < cpu_bound_procs) {
            int pid = fork();
            if (pid == 0) {
                // Executa o processo CPU-bound
                char *args[] = { "cpu_bound", 0 };
                exec("cpu_bound", args);
                exit();
            } else if (pid > 0) {
                // Armazena o PID do processo filho
                pids[pid_index] = pid;
                start_times[pid_index] = uptime(); // Registra o tempo de início
                pid_index++;
            } else {
                printf(1, "Erro ao criar processo CPU-bound\n");
            }
        }

        if (i < io_bound_procs) {
            int pid = fork();
            if (pid == 0) {
                // Executa o processo IO-bound
                char *args[] = { "io_bound", 0 };
                exec("io_bound", args);
                exit();
            } else if (pid > 0) {
                // Armazena o PID do processo filho
                pids[pid_index] = pid;
                start_times[pid_index] = uptime(); // Registra o tempo de início
                pid_index++;
            } else {
                printf(1, "Erro ao criar processo IO-bound\n");
            }
        }

        // Pausa opcional entre processos
        sleep(10);
    }

    // Espera por todos os processos filhos
    for (int i = 0; i < pid_index; i++) {
        int wait_pid = wait();

        // Encontrar o índice do processo que terminou
        int idx = -1;
        for (int j = 0; j < pid_index; j++) {
            if (pids[j] == wait_pid) {
                idx = j;
                break;
            }
        }

        if (idx != -1) {
            end_times[idx] = uptime(); // Registra o tempo de fim
            execution_times[idx] = end_times[idx] - start_times[idx]; // Calcula o tempo de execução
            total_processes_completed++;
        } else {
            printf(1, "PID desconhecido %d recebido do wait()\n", wait_pid);
        }
    }


    int end_time = uptime();
    int execution_time = end_time - start_time;

    if (execution_time > 0) {
        // Calcular o throughput da rodada
        unsigned int current_throughput = (total_processes_completed * SCALE_FACTOR) / execution_time;

        // Atualizar min, max e avg throughput
        if (min_throughput == 0 || current_throughput < min_throughput) {
            min_throughput = current_throughput;
        }
        if (current_throughput > max_throughput) {
            max_throughput = current_throughput;
        }
        // Atualizar throughput médio usando média ponderada
        if (avg_throughput == 0) {
            avg_throughput = current_throughput;
        } else {
            avg_throughput = (avg_throughput + current_throughput) / 2;
        }
    }
    if (max_throughput > min_throughput) {
        unsigned int numerator = 0;
        unsigned int denominator = max_throughput - min_throughput;

        if (denominator > 0) {
            numerator = (avg_throughput - min_throughput) * SCALE_FACTOR;
            *norm_throughput = SCALE_FACTOR - (numerator / denominator);

            if (*norm_throughput > SCALE_FACTOR) {
                *norm_throughput = SCALE_FACTOR;
            }

            printf(1, "Throughput: 0.");
            print_with_leading_zeros(*norm_throughput % SCALE_FACTOR, 6);
            printf(1, "\n");
        } else {
            printf(1, "Could not calculate Normalized Throughput\n");
        }
    } else {
        printf(1, "Could not calculate Normalized Throughput: max_throughput <= min_throughput\n");
    }
    // Collect stats from io_bound processes
    unsigned int total_write_time = 0;
    unsigned int total_read_time = 0;
    unsigned int total_delete_time = 0;
    unsigned int total_alloc_time = 0;
    unsigned int total_free_time = 0;
    int num_io_processes = 0;
    int num_cpu_processes = 0;

    for (int i = 0; i < pid_index; i++) {
        char stats_filename[32];
        build_filename(stats_filename, pids[i], "stats_");

        unsigned int write_time = 0;
        unsigned int read_time = 0;
        unsigned int delete_time = 0;
        unsigned int alloc_time = 0;
        unsigned int free_time = 0;

        // Try to read stats file from io_bound process
        read_stats(stats_filename, &write_time, &read_time, &delete_time, &alloc_time, &free_time);

        if (write_time > 0 || read_time > 0 || delete_time > 0) {
            // It's an io_bound process
            total_write_time += write_time;
            total_read_time += read_time;
            total_delete_time += delete_time;
            total_alloc_time += alloc_time;
            total_free_time += free_time;
            num_io_processes++;

            // Remove the stats file
            unlink(stats_filename);
        } else {
            // Try to read memory stats from cpu_bound process
            build_filename(stats_filename, pids[i], "stats_mem_");
            unsigned int cpu_alloc_time = 0;
            unsigned int cpu_free_time = 0;
            read_mem_stats(stats_filename, &cpu_alloc_time, &cpu_free_time);

            if (cpu_alloc_time > 0 || cpu_free_time > 0) {
                // It's a cpu_bound process
                total_alloc_time += cpu_alloc_time;
                total_free_time += cpu_free_time;
                num_cpu_processes++;

                // Remove the stats file
                unlink(stats_filename);
            }
        }
    }

    // Compute E_fs (File System Efficiency)
    if (num_io_processes > 0) {
        unsigned int total_io_time = total_write_time + total_read_time + total_delete_time;
        if (total_io_time > 0) {
            *E_fs_scaled = (1000000) / total_io_time;
            printf(1, "E_fs (File System Efficiency): 0.");
            print_with_leading_zeros(*E_fs_scaled % 1000000, 6);
            printf(1, "\n");
        } else {
            printf(1, "Total time for file system operations is zero\n");
        }
    } else {
        printf(1, "No io_bound processes stats collected\n");
    }

    // Compute M_over (Memory Management Overhead)
    unsigned int total_mem_time = total_alloc_time + total_free_time;
    if (total_mem_time > 0) {
        *M_over_scaled = (1000000) / total_mem_time;
        printf(1, "M_over (Memory Management Overhead): 0.");
        print_with_leading_zeros(*M_over_scaled % 1000000, 6);
        printf(1, "\n");
    } else {
        printf(1, "Total time for memory management is zero\n");
    }

    // Compute J_cpu (Process Fairness)
    unsigned int sum_exec_times = 0;
    unsigned int sum_exec_times_sq = 0;
    for (int i = 0; i < pid_index; i++) {
        sum_exec_times += execution_times[i];
        sum_exec_times_sq += execution_times[i] * execution_times[i];
    }

    if (sum_exec_times_sq > 0) {
        unsigned int numerator = (unsigned int)sum_exec_times * sum_exec_times * SCALE_FACTOR;
        unsigned int denominator = (unsigned int)pid_index * sum_exec_times_sq;
        if (denominator > 0) {
            *J_cpu_scaled = (unsigned int)(numerator / denominator);
            if (*J_cpu_scaled > SCALE_FACTOR) {
                *J_cpu_scaled = SCALE_FACTOR;
            }
            printf(1, "J_cpu (Process Fairness): 0.");
            print_with_leading_zeros(*J_cpu_scaled % SCALE_FACTOR, 6);
            printf(1, "\n");
        } else {
            printf(1, "Cannot compute J_cpu, denominator is zero\n");
        }
    } else {
        printf(1, "Cannot compute J_cpu, sum of execution times squared is zero\n");
    }

    // Display execution time of the round
    printf(1, "Execution time of the round: %d ticks\n", execution_time);
    printf(1, "-----------------------------------------\n");
}

int main(int argc, char *argv[]) {
    int cpu_bound_procs = 10; // Default number of CPU-bound processes

    // Check command line arguments
    if (argc > 1) {
        cpu_bound_procs = custom_atoi(argv[1]);
        if (cpu_bound_procs < 6 || cpu_bound_procs > 14) {
            printf(1, "Number of CPU-bound processes must be between 6 and 14\n");
            exit();
        }
    }

    int io_bound_procs = 20 - cpu_bound_procs;

    unsigned int J_cpu_scaled = 0;
    unsigned int E_fs_scaled = 0;
    unsigned int M_over_scaled = 0;
    unsigned int norm_throughput = 0;

    // Execute 30 rounds of the experiment
    for (int round = 0; round < 30; round++) {
        printf(1, "Round %d\n", round + 1);
        run_experiment(cpu_bound_procs, io_bound_procs, &J_cpu_scaled, &E_fs_scaled, &M_over_scaled, &norm_throughput);
    }

    // Calcular o Desempenho Geral do Sistema (S_perform)
    if (norm_throughput > 0 && J_cpu_scaled > 0 && E_fs_scaled > 0 && M_over_scaled > 0) {
        unsigned int S_perform = (norm_throughput / 4) + (J_cpu_scaled / 4) + (E_fs_scaled / 4) + (M_over_scaled / 4);
        if (S_perform > SCALE_FACTOR) {
            S_perform = SCALE_FACTOR;
        }
        printf(1, "Overall System Performance (S_perform): 0.");
        print_with_leading_zeros(S_perform % SCALE_FACTOR, 6);
        printf(1, "\n");
    } else {
        printf(1, "Could not calculate the Overall System Performance\n");
    }

    printf(1, "Experiment completed with %d processes completed\n", total_processes_completed);
    exit();
}
