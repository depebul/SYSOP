#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

volatile sig_atomic_t confirmation_received = 0;

// Handler for SIGUSR1 (confirmation from catcher)
void handle_confirmation(int signum)
{
    confirmation_received = 1;
}

// Define sigqueue manually if it's not available on the system
#ifndef __APPLE__
// Regular implementation for Linux systems
#else
// Manual implementation for macOS
int sigqueue(pid_t pid, int sig, const union sigval value)
{
    // On macOS, we can't send values with signals, so just send the signal
    return kill(pid, sig);
}
#endif

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <catcher_pid> <mode>\n", argv[0]);
        fprintf(stderr, "Modes:\n");
        fprintf(stderr, "  1 - Print count of mode change requests\n");
        fprintf(stderr, "  2 - Start counting seconds\n");
        fprintf(stderr, "  3 - Ignore Ctrl+C\n");
        fprintf(stderr, "  4 - Custom handler for Ctrl+C\n");
        fprintf(stderr, "  5 - Exit catcher\n");
        return 1;
    }

    // Parse command line arguments
    pid_t catcher_pid = atoi(argv[1]);
    int mode = atoi(argv[2]);

    if (catcher_pid <= 0)
    {
        fprintf(stderr, "Invalid catcher PID\n");
        return 1;
    }

    if (mode < 1 || mode > 5)
    {
        fprintf(stderr, "Invalid mode (must be 1-5)\n");
        return 1;
    }

    // Set up signal handler for confirmation
    signal(SIGUSR1, handle_confirmation);

    // Prepare sigqueue data
    union sigval value;
    value.sival_int = mode;

    // Block SIGUSR1 for sigsuspend
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);

    // Send the signal with mode information
    printf("Sending mode %d to catcher (PID: %d)\n", mode, catcher_pid);
    if (sigqueue(catcher_pid, SIGUSR1, value) < 0)
    {
        perror("Failed to send signal");
        return 1;
    }

    // Wait for confirmation using sigsuspend
    printf("Waiting for confirmation...\n");
    while (!confirmation_received)
    {
        sigsuspend(&old_mask);
    }

    printf("Confirmation received, exiting\n");

    // Restore original signal mask
    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    return 0;
}