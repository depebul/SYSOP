#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sighandler(int sig, siginfo_t *info, void *ucontext)
{
    int value = info->si_value.sival_int;
    printf("wartosc, %d \n", value);
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;
    action.sa_flags = SA_SIGINFO;

    //..........

    int child = fork();
    if (child == 0)
    {
        // zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        // zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        // na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        sigset_t maska;
        sigfillset(&maska);
        sigdelset(&maska, SIGUSR1);
        sigprocmask(SIG_SETMASK, &maska, NULL);

        sigemptyset(&action.sa_mask);
        sigaction(SIGUSR1, &action, NULL);
        while (1)
        {
            pause();
        }
    }
    else
    {
        // wyslij do procesu potomnego sygnal przekazany jako argv[2]
        // wraz z wartoscia przekazana jako argv[1]
        union sigval value;
        value.sival_int = atoi(argv[1]);
        if (sigqueue(child, atoi(argv[2]), value) < 0)
        {
            perror("sigqueue");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
