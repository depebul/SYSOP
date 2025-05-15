#include <stdio.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "info.h"

void sem_op(int sem_id, int sem_num, int op)
{
    struct sembuf sb = {sem_num, op, 0};
    semop(sem_id, &sb, 1);
}

void read_shift_buffer(sem_t *sem_buffer, char *buffer, char *dest)
{
    sem_wait(sem_buffer);
    strncpy(dest, buffer, MESSAGE_LENGTH);
    memmove(buffer, buffer + MESSAGE_LENGTH, strlen(buffer) - MESSAGE_LENGTH + 1);
    sem_post(sem_buffer);
}

void print_message(char *message)
{
    for (int i = 0; i < MESSAGE_LENGTH; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            printf("%c", message[j]);
        }
        printf("\n");
        sleep(1);
    }
}

int main(void)
{

    int sem_id = semget(SEM_KEY, 3, 0644);
    int shm_id = shmget(SHM_KEY, MEMORY_SIZE, 0644);
    char *buffer = shmat(shm_id, NULL, 0);
    char message[MESSAGE_LENGTH + 1];

    printf("Printer %d ready\n", getpid());

    while (1)
    {

        sem_op(sem_id, SEM_REQUEST_COUNT, -1);

        sem_op(sem_id, SEM_BUFFER_CHANGE, -1);
        strncpy(message, buffer, MESSAGE_LENGTH);
        message[MESSAGE_LENGTH] = '\0';
        memmove(buffer, buffer + MESSAGE_LENGTH, strlen(buffer) - MESSAGE_LENGTH + 1);
        sem_op(sem_id, SEM_BUFFER_CHANGE, 1);

        sem_op(sem_id, SEM_QUEUE_SPACE, 1);

        printf("Printer %d processing: %s\n", getpid(), message);

        print_message(message);
    }

    return 0;
}