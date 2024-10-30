#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define NUM_LINES 100
#define LINE_LENGTH 100

// Função simples para gerar números pseudo-aleatórios
unsigned int custom_rand() {
    static unsigned int seed = 123456789;  // Semente fixa para reproduzir os resultados
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

// Gera uma linha aleatória de 100 caracteres
void generate_random_line(char *line) {
    for (int i = 0; i < LINE_LENGTH - 1; i++) {
        line[i] = 'A' + (custom_rand() % 26);  // Letras aleatórias
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

    // Realizar permutações e exclusão do arquivo
    close(fd);
    unlink("testfile.txt");
}

int main() {
    for (int i = 0; i < 30; i++) {
        perform_io_operations();
    }
    exit();
}
