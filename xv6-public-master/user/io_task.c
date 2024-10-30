#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define FILE_NAME "iotestfile"
#define LINE_LENGTH 100
#define NUM_LINES 100

void write_lines(int fd)
{
    char buffer[LINE_LENGTH];
    for (int i = 0; i < LINE_LENGTH - 1; i++)
        buffer[i] = 'A';
    buffer[LINE_LENGTH - 1] = '\n';

    for (int i = 0; i < NUM_LINES; i++)
    {
        write(fd, buffer, LINE_LENGTH);
    }
}

int main(int argc, char *argv[])
{
    int fd = open(FILE_NAME, O_CREATE | O_RDWR);
    if (fd >= 0)
    {
        write_lines(fd);
        close(fd);
    }

    fd = open(FILE_NAME, O_RDWR);
    for (int i = 0; i < NUM_LINES / 2; i++)
    {
        int pos1 = (i * 37) % NUM_LINES;
        int pos2 = (i * 53) % NUM_LINES;
        lseek(fd, pos1 * LINE_LENGTH, SEEK_SET);
        read(fd, NULL, LINE_LENGTH);
        lseek(fd, pos2 * LINE_LENGTH, SEEK_SET);
        read(fd, NULL, LINE_LENGTH);
    }

    unlink(FILE_NAME);
    exit();
}
