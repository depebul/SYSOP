#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h> // For gettimeofday
#include <time.h>

double calculate_integral_part(double start, double end, double width)
{
    double integral_sum = 0.0;
    double x = start + width / 2.0;
    while (x < end)
    {
        integral_sum += 4.0 / (x * x + 1.0) * width;
        x += width;
    }
    return integral_sum;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Użycie: %s <szerokość_prostokąta> <maksymalna_liczba_procesów>\n", argv[0]);
        return 1;
    }

    double width = atof(argv[1]);
    int n_processes_limit = atoi(argv[2]);

    if (width <= 0 || n_processes_limit <= 0)
    {
        fprintf(stderr, "Szerokość prostokąta i maksymalna liczba procesów muszą być większe od zera.\n");
        return 1;
    }

    double lower_bound = 0.0;
    double upper_bound = 1.0;

    printf("Szerokość prostokąta (h): %f\n", width);

    for (int k = 1; k <= n_processes_limit; ++k)
    {
        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);
        pid_t pids[k];
        int pipes[k][2];
        double results[k];
        double sub_width = (upper_bound - lower_bound) / k;

        for (int i = 0; i < k; ++i)
        {
            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {
                close(pipes[i][0]);

                double process_start = lower_bound + i * sub_width;
                double process_end = lower_bound + (i + 1) * sub_width;
                double result = calculate_integral_part(process_start, process_end, width);

                if (write(pipes[i][1], &result, sizeof(double)) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                close(pipes[i][1]);
                exit(EXIT_SUCCESS);
            }
            else
            {
                pids[i] = pid;
                close(pipes[i][1]);
            }
        }

        double total_integral = 0.0;
        for (int i = 0; i < k; ++i)
        {
            if (waitpid(pids[i], NULL, 0) == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            if (read(pipes[i][0], &results[i], sizeof(double)) == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            close(pipes[i][0]);
            total_integral += results[i];
        }

        gettimeofday(&end_time, NULL);
        double execution_time = (end_time.tv_sec - start_time.tv_sec) +
                                (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        printf("Liczba procesów (k): %d, Wynik: %.8f, Czas wykonania: %.4f s\n", k, total_integral, execution_time);
    }

    return 0;
}