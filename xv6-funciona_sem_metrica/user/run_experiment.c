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

// Read stats from a file and update the accumulated times
void read_stats(const char *filename, unsigned int *write_time, unsigned int *read_time, unsigned int *delete_time) {
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
    *write_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *read_time = custom_atoi(&buffer[i]);
    while (buffer[i] != ' ' && buffer[i] != '\0') i++;
    if (buffer[i] == ' ') i++;
    *delete_time = custom_atoi(&buffer[i]);
}

// Build a filename with a given prefix and PID
void build_filename(char *filename, int pid, const char *prefix) {
    int i = 0;
    // Copy prefix to filename
    while (prefix[i] != '\0') {
        filename[i] = prefix[i];
        i++;
    }
    // Append PID to filename
    char pid_str[12];
    int pid_len = int_to_str(pid, pid_str);
    for (int j = 0; j < pid_len; j++) {
        filename[i++] = pid_str[j];
    }
    // Append ".txt" to filename
    filename[i++] = '.';
    filename[i++] = 't';
    filename[i++] = 'x';
    filename[i++] = 't';
    filename[i] = '\0';
}

// Convert integer to string and return the length (reused from io_bound.c)
int int_to_str(unsigned int num, char *str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
    } else {
        while (num > 0) {
            str[i++] = '0' + (num % 10);
            num /= 10;
        }
        reverse_string(str, i);
    }
    str[i] = '\0';
    return i;
}

// Reverse a string in-place (reused from io_bound.c)
void reverse_string(char *str, int length) {
    int i = 0;
    int j = length - 1;
    while (i < j) {
        char temp = str[i];
        str[i++] = str[j];
        str[j--] = temp;
    }
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
            pids[pid_index++] = pid;
        }
        sleep(10); // Pause between process creation
    }

    // Wait for all child processes to complete
    for (int i = 0; i < pid_index; i++) {
        wait();
        total_processes_completed++;
    }

    int end_time = uptime();
    int execution_time = end_time - start_time;

    // Collect stats from io_bound processes
    unsigned int total_write_time = 0;
    unsigned int total_read_time = 0;
    unsigned int total_delete_time = 0;
    int num_io_processes = 0;

    for (int i = 0; i < pid_index; i++) {
        char stats_filename[32];
        build_filename(stats_filename, pids[i], "stats_");

        unsigned int write_time = 0;
        unsigned int read_time = 0;
        unsigned int delete_time = 0;
        read_stats(stats_filename, &write_time, &read_time, &delete_time);

        if (write_time > 0 || read_time > 0 || delete_time > 0) {
            // It's an io_bound process
            total_write_time += write_time;
            total_read_time += read_time;
            total_delete_time += delete_time;
            num_io_processes++;

            // Remove the stats file
            unlink(stats_filename);
        }
    }

    // Compute E_fs (Eficiência do Sistema de Arquivos)
    if (num_io_processes > 0) {
        unsigned int total_time = total_write_time + total_read_time + total_delete_time;
        if (total_time > 0) {
            // Scale the result for precision since we cannot use floating-point arithmetic
            int E_fs_scaled = (1000000) / total_time; // Scale by 1,000,000
            printf(1, "Eficiência do Sistema de Arquivos (E_fs): 0.");
            print_with_leading_zeros(E_fs_scaled % 1000000, 6);
            printf(1, "\n");
        } else {
            printf(1, "Total time for file system operations is zero\n");
        }
    } else {
        printf(1, "No io_bound processes stats collected\n");
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
