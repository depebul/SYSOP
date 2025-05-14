#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "info.h"

// Only define union semun if not already defined
#if defined(__APPLE__) || defined(__FreeBSD__)
// On macOS and FreeBSD, this is already defined in the system headers
#else
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
#endif

int main()
{
    // Create semaphores
    int sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0644);

    // Initialize semaphores
    semctl(sem_id, SEM_QUEUE_SPACE, SETVAL, (union semun){.val = QUEUE_LENGTH});
    semctl(sem_id, SEM_REQUEST_COUNT, SETVAL, (union semun){.val = 0});
    semctl(sem_id, SEM_BUFFER_CHANGE, SETVAL, (union semun){.val = 1});

    // Create shared memory
    int shm_id = shmget(SHM_KEY, MEMORY_SIZE, IPC_CREAT | 0644);
    char *buffer = shmat(shm_id, NULL, 0);

    // Initialize buffer
    buffer[0] = '\0';

    // Detach shared memory
    shmdt(buffer);

    printf("Initialization complete\n");
    return 0;
}