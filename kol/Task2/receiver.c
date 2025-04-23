#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>

#define PIPE "./squareFIFO"

int main()
{
    int val = 0;
    /***********************************
     * odczytaj z potoku nazwanego PIPE zapisana tam wartosc i przypisz ja do zmiennej val
     * posprzataj
     ************************************/
    int fd = open(PIPE, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (read(fd, &val, sizeof(val)) != sizeof(val))
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("%d square is: %d\n", val, val * val);

    close(fd);

    if (unlink(PIPE) == -1)
    {
        perror("unlink");
        exit(EXIT_FAILURE);
    }
    return 0;
}
