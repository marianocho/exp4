#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

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

// Custom random number generator
unsigned int seed = 123456789;

void custom_srand(unsigned int new_seed) {
    seed = new_seed;
}

unsigned int custom_rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

// Custom atoi function to convert string to integer
int custom_atoi(char *s) {
    int n = 0;
    while (*s >= '0' && *s <= '9') {
        n = n * 10 + (*s - '0');
        s++;
    }
    return n;
}

// Print an integer with leading zeros to match a specified width
void print_with_leading_zeros(unsigned int number, int width) {
    char buffer[7]; // 6 dígitos + '\0'
    buffer[6] = '\0';
    for (int i = 5; i >= 0; i--) {
        buffer[i] = '0' + (number % 10);
        number /= 10;
    }
    printf(1, "%s", buffer);
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

// Read stats from a file
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

unsigned int calculate_scaled_ratio(unsigned int a, unsigned int b, unsigned int c, unsigned int scale) {
    // Queremos calcular (a * a * scale) / (b * c)
    // Reescrevemos como ((a * scale) / c) * (a / b)
    // Isso reduz a chance de overflow

    if (c == 0 || b == 0) {
        // Evita divisão por zero
        return 0;
    }

    unsigned int a_scaled = (a / b);
    unsigned int remainder_a = a % b;

    unsigned int temp = (a_scaled * a_scaled * scale) / c;

    // Ajuste para o resto
    unsigned int adjustment = ((2 * a_scaled * remainder_a * scale) / c) + ((remainder_a * remainder_a * scale) / (b * b * c));

    return temp + adjustment;
}