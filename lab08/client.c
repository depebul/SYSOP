#include <stdio.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "info.h"

void sem_op(int sem_id, int sem_num, int op)
{
    struct sembuf sb = {sem_num, op, 0};
    semop(sem_id, &sb, 1);
}

// Generate random message
void generate_message(char *message)
{
    for (int i = 0; i < MESSAGE_LENGTH; i++)
    {
        message[i] = 'a' + (rand() % 26);
    }
    message[MESSAGE_LENGTH] = '\0';
}

// Add message to buffer
void add_to_buffer(int sem_id, char *buffer, char *message)
{
    sem_op(sem_id, SEM_BUFFER_CHANGE, -1);
    strcat(buffer, message);
    sem_op(sem_id, SEM_BUFFER_CHANGE, 1);
}

int main()
{
    srand(time(NULL) ^ getpid());

    // Get semaphores and shared memory
    int sem_id = semget(SEM_KEY, 3, 0644);
    int shm_id = shmget(SHM_KEY, MEMORY_SIZE, 0644);
    char *buffer = shmat(shm_id, NULL, 0);

    while (1)
    {
        // Generate random message
        char message[MESSAGE_LENGTH + 1];
        generate_message(message);

        // Wait for space in queue
        sem_op(sem_id, SEM_QUEUE_SPACE, -1);

        // Add message to buffer
        add_to_buffer(sem_id, buffer, message);

        // Signal new request
        sem_op(sem_id, SEM_REQUEST_COUNT, 1);

        printf("Client %d sent: %s\n", getpid(), message);

        // Sleep random time (1-5 seconds)
        sleep(1 + rand() % 5);
    }

    return 0;
}