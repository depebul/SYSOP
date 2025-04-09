#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum)
{
    printf("Received SIGUSR1 signal (handler mode)\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <mode>\n", argv[0]);
        fprintf(stderr, "Modes: none, ignore, handler, mask\n");
        return 1;
    }

    sigset_t pending_mask;
    sigset_t mask;

    printf("PID: %d\n", getpid());

    if (strcmp(argv[1], "ignore") == 0)
    {
        printf("Setting up SIGUSR1 to be ignored\n");
        signal(SIGUSR1, SIG_IGN);
    }
    else if (strcmp(argv[1], "handler") == 0)
    {
        printf("Installing handler for SIGUSR1\n");
        signal(SIGUSR1, handler);
    }
    else if (strcmp(argv[1], "mask") == 0)
    {
        printf("Masking SIGUSR1\n");
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL);
    }
    else if (strcmp(argv[1], "none") == 0)
    {
        printf("Default behavior for SIGUSR1\n");
    }
    else
    {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        fprintf(stderr, "Modes: none, ignore, handler, mask\n");
        return 1;
    }

    printf("Raising SIGUSR1 signal...\n");
    raise(SIGUSR1);
    printf("After raise()\n");

    if (strcmp(argv[1], "mask") == 0)
    {
        sigpending(&pending_mask);
        if (sigismember(&pending_mask, SIGUSR1))
        {
            printf("SIGUSR1 is pending\n");
        }
        else
        {
            printf("SIGUSR1 is not pending\n");
        }
    }

    return 0;
}