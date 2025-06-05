#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct
{
    double start;
    double end;
    double width;
    double *result;
} ThreadData;

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

void *thread_func(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    *(data->result) = calculate_integral_part(data->start, data->end, data->width);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Użycie: %s <szerokość_prostokąta> <maksymalna_liczba_wątków>\n", argv[0]);
        return 1;
    }

    double width = atof(argv[1]);
    int n_threads_limit = atoi(argv[2]);

    if (width <= 0 || n_threads_limit <= 0)
    {
        fprintf(stderr, "Szerokość prostokąta i liczba wątków muszą być większe od zera.\n");
        return 1;
    }

    double lower_bound = 0.0;
    double upper_bound = 1.0;

    printf("Szerokość prostokąta (h): %f\n", width);

    for (int k = 1; k <= n_threads_limit; ++k)
    {
        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);

        pthread_t threads[k];
        ThreadData thread_data[k];
        double results[k];
        double sub_width = (upper_bound - lower_bound) / k;

        for (int i = 0; i < k; ++i)
        {
            thread_data[i].start = lower_bound + i * sub_width;
            thread_data[i].end = lower_bound + (i + 1) * sub_width;
            thread_data[i].width = width;
            thread_data[i].result = &results[i];

            if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0)
            {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }

        double total_integral = 0.0;
        for (int i = 0; i < k; ++i)
        {
            if (pthread_join(threads[i], NULL) != 0)
            {
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }
            total_integral += results[i];
        }

        gettimeofday(&end_time, NULL);
        double execution_time = (end_time.tv_sec - start_time.tv_sec) +
                                (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        printf("Liczba wątków (k): %d, Wynik: %.8f, Czas wykonania: %.4f s\n", k, total_integral, execution_time);
    }

    return 0;
}