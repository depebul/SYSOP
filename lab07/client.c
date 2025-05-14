#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_KEY 1234
#define MSG_INIT 1
#define MSG_DATA 2

struct message
{
    long mtype;
    int client_id;
    key_t client_queue_key;
    char text[256];
};

void receiver(key_t client_key)
{
    int client_queue_id;
    struct message received_msg;

    if ((client_queue_id = msgget(client_key, 0)) == -1)
    {
        perror("msgget (receiver)");
        exit(1);
    }

    while (1)
    {
        if (msgrcv(client_queue_id, &received_msg, sizeof(struct message) - sizeof(long), 0, 0) == -1)
        {
            perror("msgrcv (receiver)");
            exit(1);
        }
        printf("Klient %d: %s\n", received_msg.client_id, received_msg.text);
    }
}

int main()
{
    int server_queue_id;
    key_t key = SERVER_KEY;
    struct message msg;
    key_t client_key;
    int client_id = 0;
    pid_t pid;

    if ((server_queue_id = msgget(key, 0)) == -1)
    {
        perror("msgget (server)");
        exit(1);
    }

    client_key = ftok(".", getpid());
    if (msgget(client_key, IPC_CREAT | 0600) == -1)
    {
        perror("msgget (client)");
        exit(1);
    }

    msg.mtype = MSG_INIT;
    msg.client_queue_key = client_key;
    msgsnd(server_queue_id, &msg, sizeof(struct message) - sizeof(long), 0);

    struct message response;
    msgrcv(msgget(client_key, 0), &response, sizeof(struct message) - sizeof(long), 0, 0);
    client_id = response.client_id;
    printf("Otrzymano ID klienta: %d\n", client_id);

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        receiver(client_key);
        exit(0);
    }
    else
    {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = 0;
            msg.mtype = MSG_DATA;
            msg.client_id = client_id;
            strcpy(msg.text, buffer);
            msgsnd(server_queue_id, &msg, sizeof(struct message) - sizeof(long), 0);
        }
    }

    return 0;
}