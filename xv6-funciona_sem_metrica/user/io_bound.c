#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define NUM_LINES 20       // Number of lines
#define LINE_LENGTH 50     // Number of characters per line

// Custom random number generator
unsigned int custom_rand() {
    static unsigned int seed = 123456789;
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

// Generate a random line of uppercase letters
void generate_random_line(char *line) {
    for (int i = 0; i < LINE_LENGTH - 1; i++) {
        line[i] = 'A' + (custom_rand() % 26);
    }
    line[LINE_LENGTH - 1] = '\0';
}

// Reverse a string in-place
void reverse_string(char *str, int length) {
    int i = 0;
    int j = length - 1;
    while (i < j) {
        char temp = str[i];
        str[i++] = str[j];
        str[j--] = temp;
    }
}

// Convert integer to string and return the length
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

// Write accumulated times to a stats file
int write_times_to_file(const char *filename, unsigned int write_time, unsigned int read_time, unsigned int delete_time) {
    int fd = open(filename, O_CREATE | O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    char buffer[100];
    int n = 0;
    n += int_to_str(write_time, buffer + n);
    buffer[n++] = ' ';
    n += int_to_str(read_time, buffer + n);
    buffer[n++] = ' ';
    n += int_to_str(delete_time, buffer + n);
    buffer[n++] = '\n';
    write(fd, buffer, n);
    close(fd);
    return 0;
}

// Free allocated memory for lines
void free_lines(char **lines, int num_lines) {
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
}

// Perform I/O operations and accumulate times
void perform_io_operations(unsigned int *total_write_time, unsigned int *total_read_time, unsigned int *total_delete_time) {
    char filename[32];
    int pid = getpid();
    build_filename(filename, pid, "testfile_");

    int start_time, end_time;

    // Open file for writing
    start_time = uptime();
    int fd = open(filename, O_CREATE | O_RDWR);
    end_time = uptime();
    *total_write_time += (end_time - start_time);

    if (fd < 0) {
        printf(1, "[IO] Error opening the file\n");
        return;
    }

    // Allocate memory for lines
    char **lines = malloc(NUM_LINES * sizeof(char *));
    if (lines == 0) {
        printf(1, "[IO] Error allocating memory for lines\n");
        close(fd);
        return;
    }
    for (int i = 0; i < NUM_LINES; i++) {
        lines[i] = malloc(LINE_LENGTH * sizeof(char));
        if (lines[i] == 0) {
            printf(1, "[IO] Error allocating memory for line %d\n", i);
            free_lines(lines, i);
            close(fd);
            return;
        }
    }

    // Generate and write lines to file
    for (int i = 0; i < NUM_LINES; i++) {
        generate_random_line(lines[i]);
        start_time = uptime();
        write(fd, lines[i], LINE_LENGTH - 1);
        write(fd, "\n", 1);
        end_time = uptime();
        *total_write_time += (end_time - start_time);
    }
    close(fd);

    // Read from file
    start_time = uptime();
    fd = open(filename, O_RDONLY);
    end_time = uptime();
    *total_read_time += (end_time - start_time);

    if (fd < 0) {
        printf(1, "[IO] Error reopening the file\n");
        free_lines(lines, NUM_LINES);
        return;
    }

    char buffer[LINE_LENGTH];
    for (int i = 0; i < NUM_LINES; i++) {
        start_time = uptime();
        int n = read(fd, buffer, LINE_LENGTH - 1);
        end_time = uptime();
        *total_read_time += (end_time - start_time);
        if (n <= 0) {
            printf(1, "[IO] Error reading from file\n");
            break;
        }
    }
    close(fd);

    // Permute lines
    for (int i = 0; i < 10; i++) {
        int idx1 = custom_rand() % NUM_LINES;
        int idx2 = custom_rand() % NUM_LINES;
        char *temp = lines[idx1];
        lines[idx1] = lines[idx2];
        lines[idx2] = temp;
    }

    // Open file for writing permuted lines
    start_time = uptime();
    fd = open(filename, O_WRONLY);
    end_time = uptime();
    *total_write_time += (end_time - start_time);

    if (fd < 0) {
        printf(1, "[IO] Error reopening the file for writing\n");
        free_lines(lines, NUM_LINES);
        return;
    }

    // Write permuted lines
    for (int i = 0; i < NUM_LINES; i++) {
        start_time = uptime();
        write(fd, lines[i], LINE_LENGTH - 1);
        write(fd, "\n", 1);
        end_time = uptime();
        *total_write_time += (end_time - start_time);
    }
    close(fd);

    // Delete the file
    start_time = uptime();
    unlink(filename);
    end_time = uptime();
    *total_delete_time += (end_time - start_time);

    // Free allocated memory
    free_lines(lines, NUM_LINES);
}

int main() {
    unsigned int total_write_time = 0;
    unsigned int total_read_time = 0;
    unsigned int total_delete_time = 0;

    // Perform I/O operations multiple times
    for (int i = 0; i < 5; i++) {
        perform_io_operations(&total_write_time, &total_read_time, &total_delete_time);
    }

    // Write times to stats file
    char stats_filename[32];
    int pid = getpid();
    build_filename(stats_filename, pid, "stats_");

    if (write_times_to_file(stats_filename, total_write_time, total_read_time, total_delete_time) < 0) {
        printf(1, "[IO] Error writing stats file\n");
    }

    exit();
}
