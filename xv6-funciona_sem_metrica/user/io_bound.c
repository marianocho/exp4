#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define NUM_LINES 20       // Reduzido para 20 linhas
#define LINE_LENGTH 50     // Reduzido para 50 caracteres

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

// Função para construir o nome do arquivo com o PID sem snprintf e sem string.h
void build_filename(char *filename, int pid) {
    // Prefixo "testfile_"
    char prefix[] = "testfile_";
    int i = 0;

    // Copia o prefixo manualmente para filename
    while (prefix[i] != '\0') {
        filename[i] = prefix[i];
        i++;
    }

    // Converte o PID para string e adiciona ao filename
    int pid_start = i;
    while (pid > 0) {
        filename[i++] = '0' + (pid % 10);
        pid /= 10;
    }

    // Inverte a ordem dos dígitos do PID
    for (int j = 0; j < (i - pid_start) / 2; j++) {
        char temp = filename[pid_start + j];
        filename[pid_start + j] = filename[i - j - 1];
        filename[i - j - 1] = temp;
    }
    filename[i++] = '.';
    filename[i++] = 't';
    filename[i++] = 'x';
    filename[i++] = 't';
    filename[i] = '\0'; 
}

void perform_io_operations() {
    char filename[20];
    int pid = getpid();
    build_filename(filename, pid);

    int fd = open(filename, O_CREATE | O_RDWR);
    if (fd < 0) {
        printf(1, "[IO] Erro ao abrir o arquivo\n");
        return;
    }

    char **lines = malloc(NUM_LINES * sizeof(char *));
    if (lines == 0) {
        printf(1, "[IO] Erro ao alocar memória para lines\n");
        close(fd);
        return;
    }

    for (int i = 0; i < NUM_LINES; i++) {
        lines[i] = malloc(LINE_LENGTH * sizeof(char));
        if (lines[i] == 0) {
            printf(1, "[IO] Erro ao alocar memória para line %d\n", i);
            for (int j = 0; j < i; j++) {
                free(lines[j]);
            }
            free(lines);
            close(fd);
            return;
        }
    }

    for (int i = 0; i < NUM_LINES; i++) {
        generate_random_line(lines[i]);
        write(fd, lines[i], strlen(lines[i]));
        write(fd, "\n", 1);
    }

    close(fd);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf(1, "[IO] Erro ao reabrir o arquivo\n");
        for (int i = 0; i < NUM_LINES; i++) {
            free(lines[i]);
        }
        free(lines);
        return;
    }

    for (int i = 0; i < 10; i++) {
        int idx1 = custom_rand() % NUM_LINES;
        int idx2 = custom_rand() % NUM_LINES;
        char *temp = lines[idx1];
        lines[idx1] = lines[idx2];
        lines[idx2] = temp;
    }

    for (int i = 0; i < NUM_LINES; i++) {
        write(fd, lines[i], strlen(lines[i]));
        write(fd, "\n", 1);
    }

    close(fd);
    unlink(filename);

    for (int i = 0; i < NUM_LINES; i++) {
        free(lines[i]);
    }
    free(lines);
}

int main() {
    for (int i = 0; i < 5; i++) {  // Reduzido para 5 iterações
        //printf(1, "[IO] Rodada %d\n", i + 1);
        perform_io_operations();
    }
    exit();
}
