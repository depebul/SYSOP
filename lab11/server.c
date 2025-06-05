#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_NAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500
#define MAX_CLIENTS 10

typedef struct
{
    int socket;
    char name[MAX_NAME_LENGTH];
    int alive;
} client_info;

client_info clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_pinging(void *args)
{
    while (1)
    {
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].socket > 0)
            {
                if (clients[i].alive)
                {
                    clients[i].alive = 0;
                    write(clients[i].socket, "PING", 4);
                }
                else
                {
                    close(clients[i].socket);
                    clients[i].socket = 0;
                    printf("Client %s disconnected (ping timeout)\n", clients[i].name);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(5);
    }
    return NULL;
}

int add_client(int socket, char *name)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == 0)
        {
            clients[i].socket = socket;
            strcpy(clients[i].name, name);
            clients[i].alive = 1;
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1;
}

void remove_client(int index)
{
    pthread_mutex_lock(&clients_mutex);
    if (index >= 0 && index < MAX_CLIENTS)
    {
        close(clients[index].socket);
        clients[index].socket = 0;
        memset(clients[index].name, 0, MAX_NAME_LENGTH);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *args)
{
    int client_index = *(int *)args;
    free(args);

    char buffer[MAX_MESSAGE_LENGTH];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        if (read(clients[client_index].socket, buffer, MAX_MESSAGE_LENGTH) <= 0)
            break;

        if (strncmp(buffer, "LIST", 4) == 0)
        {
            char users[MAX_MESSAGE_LENGTH] = "Users:\n";
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].socket > 0)
                {
                    strcat(users, " - ");
                    strcat(users, clients[i].name);
                    strcat(users, "\n");
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            write(clients[client_index].socket, users, strlen(users));
        }
        else if (strncmp(buffer, "2ALL", 4) == 0)
        {
            char message[MAX_MESSAGE_LENGTH];
            sprintf(message, "[%s]: %s", clients[client_index].name, buffer + 5);
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].socket > 0 && i != client_index)
                {
                    write(clients[i].socket, message, strlen(message));
                }
            }
            pthread_mutex_unlock(&clients_mutex);
        }
        else if (strncmp(buffer, "2ONE", 4) == 0)
        {
            char *recipient = strtok(buffer + 5, " ");
            char *content = strtok(NULL, "");
            if (recipient && content)
            {
                char message[MAX_MESSAGE_LENGTH];
                sprintf(message, "(whisper) [%s]: %s", clients[client_index].name, content);
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i].socket > 0 && strcmp(clients[i].name, recipient) == 0)
                    {
                        write(clients[i].socket, message, strlen(message));
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);
            }
        }
        else if (strncmp(buffer, "STOP", 4) == 0)
        {
            break;
        }
        else if (strncmp(buffer, "ALIVE", 5) == 0)
        {
            clients[client_index].alive = 1;
        }
    }

    printf("Client %s disconnected\n", clients[client_index].name);
    remove_client(client_index);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    int server_socket;
    struct sockaddr_in server_address, client_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, MAX_CLIENTS);
    printf("Server started on %s:%s\n", argv[1], argv[2]);

    pthread_t pinging_thread;
    pthread_create(&pinging_thread, NULL, &handle_pinging, NULL);

    while (1)
    {
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);

        char name[MAX_NAME_LENGTH];
        memset(name, 0, MAX_NAME_LENGTH);
        int bytes_read = read(client_socket, name, MAX_NAME_LENGTH - 1);
        if (bytes_read > 0)
        {
            name[bytes_read] = '\0';
        }

        int client_index = add_client(client_socket, name);
        if (client_index == -1)
        {
            printf("Max clients reached\n");
            close(client_socket);
            continue;
        }

        printf("Client %s connected\n", name);

        int *index = malloc(sizeof(int));
        *index = client_index;
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, &handle_client, index);
        pthread_detach(client_thread);
    }

    return 0;
}