#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define MAX_CLIENTS 10
#define MAX_NAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500

struct client
{
    char name[MAX_NAME_LENGTH];
    struct sockaddr_in address;
    int active;
    time_t last_ping;
};

struct client clients[MAX_CLIENTS];
int client_count = 0;
int server_sock;
pthread_mutex_t clients_mutex;

void server_shutdown()
{
    close(server_sock);
    exit(0);
}

int find_client_by_address(struct sockaddr_in *addr)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].active &&
            clients[i].address.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].address.sin_port == addr->sin_port)
        {
            return i;
        }
    }
    return -1;
}

int find_client_by_name(char *name)
{
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].active && strcmp(clients[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void add_client(char *name, struct sockaddr_in *addr)
{
    pthread_mutex_lock(&clients_mutex);

    if (client_count < MAX_CLIENTS)
    {
        strcpy(clients[client_count].name, name);
        clients[client_count].address = *addr;
        clients[client_count].active = 1;
        clients[client_count].last_ping = time(NULL);
        client_count++;

        printf("Client %s joined the chat\n", name);
    }

    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int index)
{
    pthread_mutex_lock(&clients_mutex);

    if (index >= 0 && index < client_count && clients[index].active)
    {
        printf("Client %s left the chat\n", clients[index].name);
        clients[index].active = 0;
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_to_all(char *message, int sender_index)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].active && i != sender_index)
        {
            sendto(server_sock, message, strlen(message), 0,
                   (struct sockaddr *)&clients[i].address, sizeof(clients[i].address));
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_to_one(char *target_name, char *message, int sender_index)
{
    pthread_mutex_lock(&clients_mutex);

    int target_index = find_client_by_name(target_name);
    if (target_index != -1)
    {
        sendto(server_sock, message, strlen(message), 0,
               (struct sockaddr *)&clients[target_index].address, sizeof(clients[target_index].address));
    }
    else
    {
        char error_msg[MAX_MESSAGE_LENGTH];
        sprintf(error_msg, "User %s not found.\n", target_name);
        sendto(server_sock, error_msg, strlen(error_msg), 0,
               (struct sockaddr *)&clients[sender_index].address, sizeof(clients[sender_index].address));
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_client_list(int client_index)
{
    pthread_mutex_lock(&clients_mutex);

    char list_msg[MAX_MESSAGE_LENGTH] = "Active users:\n";
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].active)
        {
            strcat(list_msg, "- ");
            strcat(list_msg, clients[i].name);
            strcat(list_msg, "\n");
        }
    }

    sendto(server_sock, list_msg, strlen(list_msg), 0,
           (struct sockaddr *)&clients[client_index].address, sizeof(clients[client_index].address));

    pthread_mutex_unlock(&clients_mutex);
}

void *ping_clients(void *args)
{
    while (1)
    {
        sleep(30);

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < client_count; i++)
        {
            if (clients[i].active)
            {
                sendto(server_sock, "PING", 4, 0,
                       (struct sockaddr *)&clients[i].address, sizeof(clients[i].address));
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, server_shutdown);
    pthread_mutex_init(&clients_mutex, NULL);

    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Bind failed\n");
        return 1;
    }

    printf("UDP Server started on port %s\n", argv[1]);

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_clients, NULL);

    char buffer[MAX_MESSAGE_LENGTH];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recvfrom(server_sock, buffer, MAX_MESSAGE_LENGTH, 0,
                                      (struct sockaddr *)&client_address, &client_len);

        if (bytes_received > 0)
        {
            int client_index = find_client_by_address(&client_address);

            if (client_index == -1)
            {
                buffer[bytes_received] = '\0';
                add_client(buffer, &client_address);
            }
            else
            {
                clients[client_index].last_ping = time(NULL);

                if (strncmp(buffer, "2ALL", 4) == 0)
                {
                    char message[MAX_MESSAGE_LENGTH];
                    sprintf(message, "[%s]: %s", clients[client_index].name, buffer + 5);
                    send_to_all(message, client_index);
                }
                else if (strncmp(buffer, "2ONE", 4) == 0)
                {
                    char *target_name = strtok(buffer + 5, " ");
                    char *msg_content = strtok(NULL, "\0");
                    if (target_name && msg_content)
                    {
                        char message[MAX_MESSAGE_LENGTH];
                        sprintf(message, "(whisper) [%s]: %s", clients[client_index].name, msg_content);
                        send_to_one(target_name, message, client_index);
                    }
                }
                else if (strncmp(buffer, "LIST", 4) == 0)
                {
                    send_client_list(client_index);
                }
                else if (strncmp(buffer, "STOP", 4) == 0)
                {
                    remove_client(client_index);
                }
                else if (strcmp(buffer, "ALIVE") == 0)
                {
                    // Response to ping - nothing to do, already updated last_ping
                }
            }
        }
    }

    close(server_sock);
    return 0;
}