#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define NUM_LINES 100
#define LINE_LENGTH 100

unsigned int custom_rand() {
    static unsigned int seed = 123456789;
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void generate_random_line(char *line) {
    for (int i = 0; i < LINE_LENGTH - 1; i++) {
        line[i] = 'A' + (custom_rand() % 26);
    }
    line[LINE_LENGTH - 1] = '\0';
}

void perform_io_operations() {
    int fd = open("testfile.txt", O_CREATE | O_RDWR);
    char line[LINE_LENGTH];

    for (int i = 0; i < NUM_LINES; i++) {
        generate_random_line(line);
        write(fd, line, strlen(line));
    }
    
    close(fd);
    unlink("testfile.txt");
}

int main() {
    uint start_time = uptime();

    for (int i = 0; i < 30; i++) {
        perform_io_operations();
    }

    uint end_time = uptime();
    int elapsed = end_time - start_time;
    printf(1, "IO-bound tempo total: %d ticks\n", elapsed);
    exit();
}
