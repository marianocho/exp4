// run_experiments.c
#include "../types.h"
#include "../user.h"
#include "../stat.h"
#include "../fcntl.h"

#define MAX_PROCS 20  // Maximum number of processes

int total_processes_completed = 0;

// Custom atoi function to convert string to integer
int custom_atoi(char *s) {
    int n = 0;
    while (*s >= '0' && *s <= '9') {
        n = n * 10 + (*s - '0');
        s++;
    }
    return n;
}

// Update read_stats to include alloc and free times
void read_stats(const char *filename, unsigned int *write_time, unsigned int *read_time, unsigned int *delete_time, unsigned int *alloc_time, unsigned int *free_time) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return;
    }

    char buffer[150];
    int n = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (n <= 0) {
        return;
    }
    buffer[n] = '\0';

    int i = 0;
    *write_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *read_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *delete_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *alloc_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *free_time = custom_atoi(&buffer[i]);
}

// Read memory stats from a file
void read_mem_stats(const char *filename, unsigned int *alloc_time, unsigned int *free_time) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return;
    }

    char buffer[100];
    int n = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (n <= 0) {
        return;
    }
    buffer[n] = '\0';

    int i = 0;
    *alloc_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *free_time = custom_atoi(&buffer[i]);
}

// Print an integer with leading zeros to match a specified width
void print_with_leading_zeros(int number, int width) {
    char buffer[10];
    for (int i = width - 1; i >= 0; i--) {
        buffer[i] = '0' + (number % 10);
        number /= 10;
    }
    buffer[width] = '\0';
    printf(1, "%s", buffer);
}

// Main function to run the experiment
void run_experiment(int cpu_bound_procs, int io_bound_procs) {
    int start_time = uptime();

    int pids[MAX_PROCS];
    int pid_index = 0;

    int start_times[MAX_PROCS];
    int end_times[MAX_PROCS];
    int execution_times[MAX_PROCS];

    // Start CPU-bound processes
    for (int i = 0; i < cpu_bound_procs; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Error creating cpu_bound process\n");
            exit();
        }
        if (pid == 0) {
            // Child process
            char *args[] = { "cpu_bound", 0 };
            exec("cpu_bound", args);
            printf(1, "Error executing cpu_bound\n");
            exit();
        } else {
            start_times[pid_index] = uptime();  // Record start time
            pids[pid_index++] = pid;
        }
    }

    // Start IO-bound processes
    for (int i = 0; i < io_bound_procs; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Error creating io_bound process\n");
            exit();
        }
        if (pid == 0) {
            // Child process
            char *args[] = { "io_bound", 0 };
            exec("io_bound", args);
            printf(1, "Error executing io_bound\n");
            exit();
        } else {
            start_times[pid_index] = uptime();  // Record start time
            pids[pid_index++] = pid;
        }
        sleep(10); // Pause between process creation
    }

    // Wait for all child processes to complete
    for (int i = 0; i < pid_index; i++) {
        int wait_pid = wait();

        // Find the index of the completed process in pids array
        int idx = -1;
        for (int j = 0; j < pid_index; j++) {
            if (pids[j] == wait_pid) {
                idx = j;
                break;
            }
        }

        if (idx != -1) {
            end_times[idx] = uptime();          // Record end time
            execution_times[idx] = end_times[idx] - start_times[idx];  // Calculate execution time
            total_processes_completed++;
        } else {
            printf(1, "Unknown PID %d received from wait()\n", wait_pid);
        }
    }

    int end_time = uptime();
    int execution_time = end_time - start_time;

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
            int E_fs_scaled = (1000000) / total_io_time; // Scale by 1,000,000
            printf(1, "E_fs (File System Efficiency): 0.");
            print_with_leading_zeros(E_fs_scaled % 1000000, 6);
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
        int M_over_scaled = (1000000) / total_mem_time; // Scale by 1,000,000
        printf(1, "M_over (Memory Management Overhead): 0.");
        print_with_leading_zeros(M_over_scaled % 1000000, 6);
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
        double numerator = (double)(sum_exec_times * sum_exec_times);
        double denominator = (double)(pid_index * sum_exec_times_sq);
        double J_cpu = numerator / denominator;
        printf(1, "J_cpu (Process Fairness): ");
        int J_cpu_scaled = (int)(J_cpu * 1000000); // Scale to 6 decimal places
        printf(1, "0.");
        print_with_leading_zeros(J_cpu_scaled % 1000000, 6);
        printf(1, "\n");
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

    // Execute 30 rounds of the experiment
    for (int round = 0; round < 30; round++) {
        printf(1, "Round %d\n", round + 1);
        run_experiment(cpu_bound_procs, io_bound_procs);
    }
    printf(1, "Experiment completed with %d processes completed\n", total_processes_completed);
    exit();
}
