// run_experiments.c
#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"
#include "utils.h"

#define UINT_MAX ((unsigned int)~0)
#define MAX_PROCS 20  // Maximum number of processes
#define SCALE_FACTOR 1000000
unsigned int min_throughput = UINT_MAX;
unsigned int max_throughput = 0;

// Função para executar uma rodada do experimento
void run_experiment(int cpu_bound_procs, int io_bound_procs, unsigned int *J_cpu_scaled, unsigned int *E_fs_scaled, unsigned int *M_over_scaled, unsigned int *norm_throughput) {
    int start_time = uptime();

    unsigned int total_processes_completed = 0; // Variável local

    int pids[MAX_PROCS];
    int pid_index = 0;

    int start_times[MAX_PROCS];
    int end_times[MAX_PROCS];
    int execution_times[MAX_PROCS];

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
                if (pid_index < MAX_PROCS) {
                    pids[pid_index] = pid;
                    start_times[pid_index] = uptime(); // Registra o tempo de início
                    pid_index++;
                } else {
                    printf(1, "Número máximo de processos atingido\n");
                }
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
                if (pid_index < MAX_PROCS) {
                    pids[pid_index] = pid;
                    start_times[pid_index] = uptime(); // Registra o tempo de início
                    pid_index++;
                } else {
                    printf(1, "Número máximo de processos atingido\n");
                }
            } else {
                printf(1, "Erro ao criar processo IO-bound\n");
            }
        }

        // Pausa opcional entre processos
        sleep(20);
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

    // Coletar estatísticas dos processos IO-bound e CPU-bound
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

        // Tentar ler o arquivo de estatísticas do processo IO-bound
        read_stats(stats_filename, &write_time, &read_time, &delete_time, &alloc_time, &free_time);

        if (write_time > 0 || read_time > 0 || delete_time > 0) {
            // É um processo IO-bound
            total_write_time += write_time;
            total_read_time += read_time;
            total_delete_time += delete_time;
            total_alloc_time += alloc_time;
            total_free_time += free_time;
            num_io_processes++;

            // Remover o arquivo de estatísticas
            unlink(stats_filename);
        } else {
            // Tentar ler estatísticas de memória do processo CPU-bound
            build_filename(stats_filename, pids[i], "stats_mem_");
            unsigned int cpu_alloc_time = 0;
            unsigned int cpu_free_time = 0;
            read_mem_stats(stats_filename, &cpu_alloc_time, &cpu_free_time);

            if (cpu_alloc_time > 0 || cpu_free_time > 0) {
                // É um processo CPU-bound
                total_alloc_time += cpu_alloc_time;
                total_free_time += cpu_free_time;
                num_cpu_processes++;

                // Remover o arquivo de estatísticas
                unlink(stats_filename);
            }
        }
    }

    //Vazao (Throughput)
    unsigned int current_throughput_scaled = 0;
    if (execution_time > 0) {
        current_throughput_scaled = (total_processes_completed * SCALE_FACTOR) / execution_time;
    } else {
        printf(1, "Execution time is zero, cannot calculate throughput\n");
    }
    if (current_throughput_scaled < min_throughput) {
        min_throughput = current_throughput_scaled;
    }
    if (current_throughput_scaled > max_throughput) {
        max_throughput = current_throughput_scaled;
    }
    if (max_throughput > min_throughput) {
        unsigned int numerator = (current_throughput_scaled - min_throughput) * SCALE_FACTOR;
        unsigned int denominator = max_throughput - min_throughput;

        if (denominator > 0) {
            *norm_throughput = SCALE_FACTOR - (numerator / denominator);
            if (*norm_throughput > SCALE_FACTOR) {
                *norm_throughput = SCALE_FACTOR; // Limitar o valor máximo a 1.0 escalonado
            }
            printf(1, "Normalized Throughput: 0.");
            print_with_leading_zeros(*norm_throughput % SCALE_FACTOR, 6);
            printf(1, "\n");
        } else {
            printf(1, "Denominator for throughput normalization is zero\n");
        }
    }

    // Calcular E_fs (Eficiência do Sistema de Arquivos)
    if (num_io_processes > 0) {
        unsigned int total_io_time = total_write_time + total_read_time + total_delete_time;
        if (total_io_time > 0) {
            *E_fs_scaled = SCALE_FACTOR / total_io_time;
            printf(1, "E_fs (File System Efficiency): 0.");
            print_with_leading_zeros(*E_fs_scaled % SCALE_FACTOR, 6); 
            printf(1, "\n");
            // printf(1, "Debug: E_fs_scaled raw value: %u\n", *E_fs_scaled);
        } else {
            printf(1, "Total time for file system operations is zero\n");
        }
    } else {
        printf(1, "No io_bound processes stats collected\n");
    }

    // Calcular M_over (Overhead de Gerenciamento de Memória)
    unsigned int total_mem_time = total_alloc_time + total_free_time;
    if (total_mem_time > 0) {
        *M_over_scaled = SCALE_FACTOR / total_mem_time;
        printf(1, "M_over (Memory Management Overhead): 0.");
        print_with_leading_zeros(*M_over_scaled % SCALE_FACTOR, 6);  
        printf(1, "\n");
        // printf(1, "Debug: M_over_scaled raw value: %u\n", *M_over_scaled);
    } else {
        printf(1, "Total time for memory management is zero\n");
    }

    // Calcular J_cpu (Equidade de Processos)
    unsigned int sum_exec_times = 0;
    unsigned int sum_exec_times_sq = 0;
    for (int i = 0; i < pid_index; i++) {
        sum_exec_times += execution_times[i];
        sum_exec_times_sq += execution_times[i] * execution_times[i];
    }

    if (sum_exec_times_sq > 0) {
        unsigned int sum_exec_times_scaled = sum_exec_times * SCALE_FACTOR;
        unsigned int numerator = sum_exec_times * sum_exec_times_scaled;
        unsigned int denominator = pid_index * sum_exec_times_sq;

        if (denominator > 0) {
            *J_cpu_scaled = numerator / denominator;
            if (*J_cpu_scaled > SCALE_FACTOR) {
                *J_cpu_scaled = SCALE_FACTOR;
            }
            printf(1, "J_cpu (Process Fairness): 0.");
            print_with_leading_zeros(*J_cpu_scaled % SCALE_FACTOR, 6);  
            printf(1, "\n");
            // printf(1, "Debug: J_cpu_scaled raw value: %u\n", *J_cpu_scaled);
        } else {
            printf(1, "Cannot compute J_cpu, denominator is zero\n");
        }
    } else {
        printf(1, "Cannot compute J_cpu, sum of execution times squared is zero\n");
    }

    // Exibir tempo de execução da rodada
    printf(1, "Execution time of the round: %d ticks\n", execution_time);
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

    // Executar 30 rodadas do experimento
    for (int round = 0; round < 30; round++) {
        printf(1, "Round %d\n", round + 1);
        run_experiment(cpu_bound_procs, io_bound_procs, &J_cpu_scaled, &E_fs_scaled, &M_over_scaled, &norm_throughput);

        // Calcular o Desempenho Geral do Sistema (S_perform) para a rodada atual
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

        printf(1, "-----------------------------------------\n");
    }
    exit();
}