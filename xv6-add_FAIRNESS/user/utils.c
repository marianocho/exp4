#include "../types.h"
#include "../user.h"

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
