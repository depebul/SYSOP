#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

// Global variables
volatile sig_atomic_t mode = 0;
volatile sig_atomic_t sender_pid = 0;
volatile sig_atomic_t should_exit = 0;
volatile sig_atomic_t count_mode_changes = 0;
volatile sig_atomic_t counting_active = 0;

void handle_sigint(int signum)
{
    printf("Wciśnięto CTRL+C\n");
}

void handle_sigusr1(int signum, siginfo_t *info, void *context)
{

    sender_pid = info->si_pid;

    if (info->si_code == SI_QUEUE)
    {
        int new_mode = info->si_value.sival_int;
        count_mode_changes++;

        counting_active = 0;

        switch (new_mode)
        {
        case 1:

            printf("Mode 1: Received %d mode change requests so far\n", count_mode_changes);
            break;

        case 2:

            printf("Mode 2: Starting to count seconds\n");
            counting_active = 1;
            break;

        case 3:

            printf("Mode 3: Ignoring Ctrl+C now\n");
            signal(SIGINT, SIG_IGN);
            break;

        case 4:

            printf("Mode 4: Installing custom Ctrl+C handler\n");
            signal(SIGINT, handle_sigint);
            break;

        case 5:

            printf("Mode 5: Exiting program\n");
            should_exit = 1;
            break;

        default:
            printf("Unknown mode: %d\n", new_mode);
        }

        mode = new_mode;
    }

    if (sender_pid > 0)
    {
        kill(sender_pid, SIGUSR1);
    }
}

int main()
{

    printf("Catcher PID: %d\n", getpid());

    struct sigaction sa;
    sa.sa_sigaction = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("Cannot set signal handler");
        return 1;
    }

    signal(SIGINT, SIG_DFL);

    int counter = 0;
    while (!should_exit)
    {
        if (counting_active)
        {
            printf("Counter: %d\n", counter++);
            sleep(1);
        }
        else
        {

            pause();
        }
    }

    printf("Catcher terminating\n");
    return 0;
}