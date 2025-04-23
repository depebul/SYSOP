#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_PATH "/tmp/my_fifo"
#define WIDTH 0.00001

double calculate_integral(double lower_bound, double upper_bound, double width)
{
    double integral_sum = 0.0;
    double x = lower_bound + width / 2.0;
    while (x < upper_bound)
    {
        integral_sum += 4.0 / (x * x + 1.0) * width;
        x += width;
    }
    return integral_sum;
}

int main()
{
    int fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1)
    {
        perror("open (read)");
        fprintf(stderr, "Upewnij się, że pierwszy program został uruchomiony i utworzył potok.\n");
        return 1;
    }

    char buffer[50];
    ssize_t bytes_read = read(fifo_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1)
    {
        perror("read");
        close(fifo_fd);
        return 1;
    }
    buffer[bytes_read] = '\0';

    double lower_bound, upper_bound;
    if (sscanf(buffer, "%lf,%lf", &lower_bound, &upper_bound) != 2)
    {
        fprintf(stderr, "Otrzymano nieprawidłowe dane z potoku.\n");
        close(fifo_fd);
        return 1;
    }

    double result = calculate_integral(lower_bound, upper_bound, WIDTH);
    printf("Obliczona wartość całki w przedziale [%.8f, %.8f]: %.8f\n", lower_bound, upper_bound, result);

    close(fifo_fd);
    return 0;
}