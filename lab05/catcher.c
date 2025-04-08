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

// Custom Ctrl+C (SIGINT) handler
void handle_sigint(int signum)
{
    printf("Wciśnięto CTRL+C\n");
}

// Handler for SIGUSR1
void handle_sigusr1(int signum, siginfo_t *info, void *context)
{
    // Get sender PID from siginfo_t
    sender_pid = info->si_pid;

    // Process signal value (mode)
    if (info->si_code == SI_QUEUE)
    {
        int new_mode = info->si_value.sival_int;
        count_mode_changes++;

        // Stop any counting activity when changing modes
        counting_active = 0;

        switch (new_mode)
        {
        case 1:
            // Print count of mode change requests
            printf("Mode 1: Received %d mode change requests so far\n", count_mode_changes);
            break;

        case 2:
            // Start counting mode
            printf("Mode 2: Starting to count seconds\n");
            counting_active = 1;
            break;

        case 3:
            // Ignore Ctrl+C
            printf("Mode 3: Ignoring Ctrl+C now\n");
            signal(SIGINT, SIG_IGN);
            break;

        case 4:
            // Custom handler for Ctrl+C
            printf("Mode 4: Installing custom Ctrl+C handler\n");
            signal(SIGINT, handle_sigint);
            break;

        case 5:
            // Exit program
            printf("Mode 5: Exiting program\n");
            should_exit = 1;
            break;

        default:
            printf("Unknown mode: %d\n", new_mode);
        }

        mode = new_mode;
    }

    // Send confirmation back to sender
    if (sender_pid > 0)
    {
        kill(sender_pid, SIGUSR1);
    }
}

int main()
{
    // Print PID so sender can use it
    printf("Catcher PID: %d\n", getpid());

    // Set up signal handling
    struct sigaction sa;
    sa.sa_sigaction = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO; // Use sa_sigaction instead of sa_handler

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("Cannot set signal handler");
        return 1;
    }

    // Default SIGINT behavior
    signal(SIGINT, SIG_DFL);

    // Main loop
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
            // Just wait for signals
            pause();
        }
    }

    printf("Catcher terminating\n");
    return 0;
}