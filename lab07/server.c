#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CLIENTS 60
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

int client_queues[MAX_CLIENTS];
int next_client_id = 1;

int main()
{
    int server_queue_id;
    key_t key = SERVER_KEY;
    struct message msg;
    int i;

    if ((server_queue_id = msgget(key, IPC_CREAT | 0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    printf("Serwer uruchomiony. Oczekiwanie na klientów...\n");

    while (1)
    {
        if (msgrcv(server_queue_id, &msg, sizeof(struct message) - sizeof(long), 0, 0) == -1)
        {
            perror("msgrcv");
            continue;
        }

        if (msg.mtype == MSG_INIT)
        {
            if (next_client_id <= MAX_CLIENTS)
            {
                int client_queue_id;
                if ((client_queue_id = msgget(msg.client_queue_key, 0)) == -1)
                {
                    perror("msgget (client)");
                    continue;
                }
                client_queues[next_client_id - 1] = client_queue_id;
                struct message response;
                response.mtype = 1;
                response.client_id = next_client_id;
                msgsnd(client_queue_id, &response, sizeof(struct message) - sizeof(long), 0);
                printf("Klient %d połączony.\n", next_client_id);
                next_client_id++;
            }
            else
            {
                printf("Osiągnięto maksymalną liczbę klientów.\n");
            }
        }
        else if (msg.mtype == MSG_DATA)
        {
            printf("Odebrano od klienta %d: %s\n", msg.client_id, msg.text);
            for (i = 0; i < MAX_CLIENTS; ++i)
            {
                if (client_queues[i] != 0 && (i + 1) != msg.client_id)
                {
                    msg.mtype = 1;
                    msgsnd(client_queues[i], &msg, sizeof(struct message) - sizeof(long), 0);
                }
            }
        }
    }

    return 0;
}