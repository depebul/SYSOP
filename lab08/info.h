#ifndef INFO_H
#define INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

#define QUEUE_LENGTH 5
#define MESSAGE_LENGTH 10
#define MEMORY_SIZE 1024

#define SHM_KEY 12345
#define SEM_KEY 12346

#define SEM_QUEUE_SPACE 0
#define SEM_REQUEST_COUNT 1
#define SEM_BUFFER_CHANGE 2

#endif