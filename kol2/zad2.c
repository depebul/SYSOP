
#include "zad2.h"

void mask()
{
    /*  Zamaskuj sygnał SIGUSR2, tak aby nie docierał on do procesu */
    sigset_t maska;
    sigemptyset(&maska);
    sigaddset(&maska, SIGUSR2);
    sigprocmask(SIG_SETMASK, &maska, NULL);
    check_mask();
}

void process()
{
    /*  Stworz nowy process potomny i uruchom w nim program ./check_fork
        W procesie macierzystym:
            1. poczekaj 1 sekundę
            2. wyślij SIGUSR1 do procesu potomnego
            3. poczekaj na zakończenie procesu potomnego */
    pid_t child = fork();
    if (child == 0)
    {
        execl("./check_fork", NULL);
    }
    else
    {
        sleep(1);
        kill(child, SIGUSR1);
        wait(NULL);
    }
}

int main()
{
    mask();
    process();

    return 0;
}