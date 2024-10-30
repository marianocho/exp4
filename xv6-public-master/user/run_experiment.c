#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_ROUNDS 30
#define NUM_CPU_TASKS 10
#define NUM_IO_TASKS 10

void run_round()
{
    int i;

    // Executa processos CPU-bound
    for (i = 0; i < NUM_CPU_TASKS; i++)
    {
        if (fork() == 0)
        {
            exec("cpu_task", 0);
            exit();
        }
    }

    // Executa processos IO-bound
    for (i = 0; i < NUM_IO_TASKS; i++)
    {
        if (fork() == 0)
        {
            exec("io_task", 0);
            exit();
        }
    }

    for (i = 0; i < NUM_CPU_TASKS + NUM_IO_TASKS; i++)
    {
        wait();
    }
}

int main(int argc, char *argv[])
{
    for (int round = 0; round < NUM_ROUNDS; round++)
    {
        printf(1, "Rodada %d\n", round);
        run_round();
    }
    exit();
}
