#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define NUM_LINES 20       // Number of lines
#define LINE_LENGTH 50     // Number of characters per line

// Generate a random line of uppercase letters
void generate_random_line(char *line) {
    for (int i = 0; i < LINE_LENGTH - 1; i++) {
        line[i] = 'A' + (custom_rand() % 26);
    }
    line[LINE_LENGTH - 1] = '\0';
}

// Modify write_times_to_file to include alloc and free times
int write_times_to_file(const char *filename, unsigned int write_time, unsigned int read_time, unsigned int delete_time, unsigned int alloc_time, unsigned int free_time) {
    int fd = open(filename, O_CREATE | O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    char buffer[150];
    int n = 0;
    n += int_to_str(write_time, buffer + n);
    buffer[n++] = ' ';
    n += int_to_str(read_time, buffer + n);
    buffer[n++] = ' ';
    n += int_to_str(delete_time, buffer + n);
    buffer[n++] = ' ';
    n += int_to_str(alloc_time, buffer + n);
    buffer[n++] = ' ';
    n += int_to_str(free_time, buffer + n);
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
void perform_io_operations(unsigned int *total_write_time, unsigned int *total_read_time, unsigned int *total_delete_time, unsigned int *total_alloc_time, unsigned int *total_free_time) {
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
    start_time = uptime();
    char **lines = malloc(NUM_LINES * sizeof(char *));
    end_time = uptime();
    *total_alloc_time += (end_time - start_time);

    if (lines == 0) {
        printf(1, "[IO] Error allocating memory for lines\n");
        close(fd);
        return;
    }

    for (int i = 0; i < NUM_LINES; i++) {
        start_time = uptime();
        lines[i] = malloc(LINE_LENGTH * sizeof(char));
        end_time = uptime();
        *total_alloc_time += (end_time - start_time);

        if (lines[i] == 0) {
            printf(1, "[IO] Error allocating memory for line %d\n", i);
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                start_time = uptime();
                free(lines[j]);
                end_time = uptime();
                *total_free_time += (end_time - start_time);
            }
            start_time = uptime();
            free(lines);
            end_time = uptime();
            *total_free_time += (end_time - start_time);
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

    for (int i = 0; i < NUM_LINES; i++) {
        start_time = uptime();
        free(lines[i]);
        end_time = uptime();
        *total_free_time += (end_time - start_time);
    }
    start_time = uptime();
    free(lines);
    end_time = uptime();
    *total_free_time += (end_time - start_time);
}

int main() {
    // Optionally set the seed
    // custom_srand(123456789);

    unsigned int total_write_time = 0;
    unsigned int total_read_time = 0;
    unsigned int total_delete_time = 0;
    unsigned int total_alloc_time = 0;
    unsigned int total_free_time = 0;

    // Perform I/O operations multiple times
    for (int i = 0; i < 5; i++) {
        perform_io_operations(&total_write_time, &total_read_time, &total_delete_time, &total_alloc_time, &total_free_time);
    }

    // Write times to stats file
    char stats_filename[32];
    int pid = getpid();
    build_filename(stats_filename, pid, "stats_");

    if (write_times_to_file(stats_filename, total_write_time, total_read_time, total_delete_time, total_alloc_time, total_free_time) < 0) {
        printf(1, "[IO] Error writing stats file\n");
    }

    exit();
}
