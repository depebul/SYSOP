#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global = 2136;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Użycie: %s <ścieżka_katalogu>\n", argv[0]);
        return 1;
    }

    int local = 2136;
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {

        printf("child process\n");
        global++;
        local++;
        printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
        printf("child's local = %d, child's global = %d\n", local, global);

        execl("/bin/ls", "ls", argv[1], NULL);
        perror("execl");
                return 1;
    }
    else
    {

        int status;
        wait(&status);
        printf("parent process\n");
        printf("parent pid = %d, child pid = %d\n", getpid(), pid);
        printf("Child exit code: %d\n", WEXITSTATUS(status));
        printf("Parent's local = %d, parent's global = %d\n", local, global);
    }
    printf("koniec\n");
    return 0;
}