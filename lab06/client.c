#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define FIFO_PATH "/tmp/my_fifo"

int main()
{
    if (mkfifo(FIFO_PATH, 0666) == -1)
    {
        perror("mkfifo");

        if (errno != EEXIST)
        {
            return 1;
        }
    }
    else
    {
        printf("Utworzono potok nazwany: %s\n", FIFO_PATH);
    }

    double lower_bound, upper_bound;

    printf("Podaj dolną granicę przedziału: ");
    if (scanf("%lf", &lower_bound) != 1)
    {
        fprintf(stderr, "Nieprawidłowe dane wejściowe.\n");
        unlink(FIFO_PATH);
        return 1;
    }

    printf("Podaj górną granicę przedziału: ");
    if (scanf("%lf", &upper_bound) != 1)
    {
        fprintf(stderr, "Nieprawidłowe dane wejściowe.\n");
        unlink(FIFO_PATH);
        return 1;
    }

    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1)
    {
        perror("open (write)");
        return 1;
    }

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%lf,%lf\n", lower_bound, upper_bound);

    if (write(fifo_fd, buffer, strlen(buffer)) == -1)
    {
        perror("write");
        close(fifo_fd);
        return 1;
    }

    printf("Wysłano przedział [%.2f, %.2f] do drugiego programu.\n", lower_bound, upper_bound);

    close(fifo_fd);
    return 0;
}