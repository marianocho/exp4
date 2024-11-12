#include "../types.h"
#include "../user.h"
#include "../fcntl.h"
#include "../stat.h"

#define NUM_LINES 100
#define LINE_LENGTH 100

// Função simples para gerar números pseudo-aleatórios
unsigned int custom_rand()
{
    static unsigned int seed = 123456789; // Semente fixa para reproduzir os resultados
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

// Gera uma linha aleatória de 100 caracteres
void generate_random_line(char *line)
{
    for (int i = 0; i < LINE_LENGTH - 1; i++)
    {
        line[i] = 'A' + (custom_rand() % 26); // Letras aleatórias
    }
    line[LINE_LENGTH - 1] = '\0';
}

void perform_io_operations()
{
    int fd = open("testfile.txt", O_CREATE | O_RDWR);
    char line[LINE_LENGTH];
    int t_start, t_end;
    int t_write, t_read, t_delete;
    int t_total;

    for (int i = 0; i < NUM_LINES; i++)
    {
        generate_random_line(line);
        // Calculando tempo de escrita
        t_start = uptime();
        write(fd, line, strlen(line));
        t_end = uptime();
        t_write += t_end - t_start;
    }

    lseek(fd, 0, 0);
    for (int i = 0; i < NUM_LINES; i++) {
        // Calculando tempo de leitura
        t_start = uptime();
        read(fd, line, LINE_LENGTH);
        t_end = uptime();
        total_read_time += t_end - t_start;
    }

    // Realizar permutações e exclusão do arquivo e calcular tempo de deleção
    close(fd);
    t_start = uptime();
    unlink("testfile.txt");
    t_end = uptime();
    t_delete += t_end - t_start;

    int avg_write = t_write / NUM_LINES;
    int avg_read = t_read / NUM_LINES;

    t_total = avg_write + avg_read + t_delete;
    float efficiency = 1.0/t_total;

    printf(1, "Tempo médio de escrita: %d ticks\n", avg_write);
    printf(1, "Tempo médio de leitura: %d ticks\n", avg_read);
    printf(1, "Tempo de deleção: %d ticks\n", t_delete);
    printf(1, "Eficiência do sistema de arquivos (E_fs): %.4f\n", efficiency);
}

int main()
{
    for (int i = 0; i < 30; i++)
    {
        perform_io_operations();
    }
    exit();
}
