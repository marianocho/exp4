#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"

int run_experiment(int cpu_bound_procs, int io_bound_procs, int *total_exec_times)
{
    int t_start, t_end;
    int total_process = cpu_bound_procs + io_bound_procs;
    int pid;

    t_start = uptime();
    for (int i = 0; i < cpu_bound_procs; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            exec("cpu_bound", 0);
            exit();
        }
        total_exec_times[i] = uptime() - t_start;
    }

    for (int i = 0; i < io_bound_procs; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            exec("io_bound", 0);
            exit();
        }
        total_exec_times[i] = uptime() - t_start;
    }

    // Esperar processos filhos finalizarem e coletar métricas (latência de IO, vazão, etc.)
    for (int i = 0; i < cpu_bound_procs + io_bound_procs; i++)
    {
        wait();
    }

    t_end = uptime();
    int time = t_end - t_start;
    int throughput = (time > 0 ? (total_process * 100) / time : 0);
    // Calcular métricas e exibir o desempenho geral do sistema
    printf(1, "Rodada concluída\n");
    return throughput;
}

float fairness(int *exec_times, int total_process)
{
    int sum_x = 0;
    int sum_x_squared = 0;

    for (int i = 0; i < total_process; i++)
    {
        sum_x += exec_times[i];
        sum_x_squared += exec_times[i] * exec_times[i];
    }
    float fair = (float)(sum_x*sum_x) / (total_processes * sum_x_squared);
    return fair;
}

int main(int argc, char *argv[])
{
    int cpu_bound_procs = atoi(argv[1]);
    int io_bound_procs = 20 - cpu_bound_procs;
    int min_throughput = 1000000;
    int max_throughput = 0;
    int total_processes = cpu_bound_procs + io_bound_procs;
    int exec_times[total_processes];
    int sum_throughput = 0;
    int throughput;
    float fair;

    for (int round = 0; round < 30; round++)
    {
        printf(1, "Rodada %d\n", round + 1);
        throughput = run_experiment(cpu_bound_procs, io_bound_procs, exec_times);

        // Throughput
        if (throughput < min_throughput)
        {
            min_throughput = throughput;
        }
        if (throughput > max_throughput)
        {
            max_throughput = throughput;
        }
        sum_throughput += throughput;

        // Fairness
        fair = fairness(exec_times, total_processes);
        printf(1, "Justiça entre processos (J_cpu): %.4f\n", fair);
    }

    float mean_throughput = (float)sum_throughput/30;
    float normalized_throughput = 1.0 - ((mean_throughput - min_throughput) / (max_throughput - min_throughput));
    printf(1, "Vazão Normalizada: %.2f\n", normalized_throughput);
    exit();
}
