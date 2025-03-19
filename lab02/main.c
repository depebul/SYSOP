#include <stdio.h>

#ifdef USE_DYNAMIC
#include <dlfcn.h>
#else

int collatz_conjecture(int input);
int test_collatz_convergence(int input, int max_iter, int *steps);
#endif

int main()
{
    int inputs[] = {12, 17, 25, 32, 41};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    int max_iter = 100;
    int steps[max_iter];
    int num_steps;

#ifdef USE_DYNAMIC
    void *handle = dlopen("./libcollatz.so", RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return 1;
    }

    int (*test_func)(int, int, int *) = dlsym(handle, "test_collatz_convergence");
    if (!test_func)
    {
        fprintf(stderr, "Symbol not found: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
#endif

    for (int i = 0; i < num_inputs; i++)
    {
        int input = inputs[i];

#ifdef USE_DYNAMIC
        num_steps = test_func(input, max_iter, steps);
#else
        num_steps = test_collatz_convergence(input, max_iter, steps);
#endif

        if (num_steps > 0)
        {
            printf("Sequence for input %d: ", input);
            for (int j = 0; j < num_steps; j++)
            {
                printf("%d ", steps[j]);
            }
            printf("\n");
        }
        else
        {
            printf("Failed to converge to 1 for %d within %d iterations.\n", input, max_iter);
        }
    }

#ifdef USE_DYNAMIC
    dlclose(handle);
#endif

    return 0;
}