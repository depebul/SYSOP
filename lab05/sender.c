#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

volatile sig_atomic_t confirmation_received = 0;

void handle_confirmation(int signum)
{
    confirmation_received = 1;
}

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

    signal(SIGUSR1, handle_confirmation);

    union sigval value;
    value.sival_int = mode;

    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);

    printf("Sending mode %d to catcher (PID: %d)\n", mode, catcher_pid);
    if (sigqueue(catcher_pid, SIGUSR1, value) < 0)
    {
        perror("Failed to send signal");
        return 1;
    }

    printf("Waiting for confirmation...\n");
    while (!confirmation_received)
    {
        sigsuspend(&old_mask);
    }

    printf("Confirmation received, exiting\n");

    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    return 0;
}