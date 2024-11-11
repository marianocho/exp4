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
    int t_start, t_end, lat;
    int total_lat = 0 int min_lat = 1000000;
    int max_lat = 0;
    int latencies[NUM_LINES];

    for (int i = 0; i < NUM_LINES; i++)
    {
        generate_random_line(line);

        // Calculando tempos
        t_start = uptime();
        write(fd, line, strlen(line));
        t_end = uptime();
        lat = t_end - t_start;
        latencies[i] = lat;
        total_lat += lat;

        // Atualizando latencias max e min
        if (lat > max_lat)
        {
            max_lat = lat;
        }
        if (lat < min_lat)
        {
            min_lat = lat;
        }
    }

    // Realizar permutações e exclusão do arquivo
    close(fd);
    unlink("testfile.txt");

    //
    int avg_lat = total_lat / NUM_LINES;
    for (int i = 0; i < NUM_LINES; i++)
    {
        float normal_lat = 1.0 - ((float)(latencies[i] - min_lat) / (max_lat - min_lat));
        printf(1, "Operação %d: Latência Normalizada = %d -> %.2f\n", i + 1, latencies[i], normalized_latency);
    }
}

int main()
{
    for (int i = 0; i < 30; i++)
    {
        perform_io_operations();
    }
    exit();
}
