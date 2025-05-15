#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "info.h"

int main()
{
    int sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0644);

    semctl(sem_id, SEM_QUEUE_SPACE, SETVAL, QUEUE_LENGTH);
    semctl(sem_id, SEM_REQUEST_COUNT, SETVAL, 0);
    semctl(sem_id, SEM_BUFFER_CHANGE, SETVAL, 1);

    int shm_id = shmget(SHM_KEY, MEMORY_SIZE, IPC_CREAT | 0644);
    char *buffer = shmat(shm_id, NULL, 0);

    buffer[0] = '\0';

    shmdt(buffer);

    printf("Initialization complete\n");
    return 0;
}