#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_NAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500

int sock;
struct sockaddr_in server_address;

void client_leave()
{
    sendto(sock, "STOP\n", 5, 0, (struct sockaddr *)&server_address, sizeof(server_address));
    close(sock);
    exit(0);
}

void *receive_handler(void *args)
{
    char message[MAX_MESSAGE_LENGTH];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (1)
    {
        memset(message, 0, sizeof(message));
        int bytes_received = recvfrom(sock, message, MAX_MESSAGE_LENGTH, 0,
                                      (struct sockaddr *)&from_addr, &from_len);
        if (bytes_received <= 0)
            break;

        if (strcmp(message, "PING") == 0)
        {
            sendto(sock, "ALIVE", 5, 0, (struct sockaddr *)&server_address, sizeof(server_address));
        }
        else
        {
            printf("%s", message);
        }
    }
    return NULL;
}

void *send_handler(void *args)
{
    char buffer[MAX_MESSAGE_LENGTH];
    char message[MAX_MESSAGE_LENGTH];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, MAX_MESSAGE_LENGTH, stdin);

        if (strncmp(buffer, "/list", 5) == 0)
        {
            strcpy(message, "LIST\n");
        }
        else if (strncmp(buffer, "/msg", 4) == 0)
        {
            sprintf(message, "2ONE %s", buffer + 5);
        }
        else if (strncmp(buffer, "/stop", 5) == 0)
        {
            client_leave();
            break;
        }
        else
        {
            sprintf(message, "2ALL %s", buffer);
        }

        sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <name> <ip> <port>\n", argv[0]);
        return 1;
    }

    signal(SIGINT, client_leave);

    // Tworzenie socketu UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[3]));
    server_address.sin_addr.s_addr = inet_addr(argv[2]);

    // Wysłanie nazwy użytkownika do serwera
    sendto(sock, argv[1], strlen(argv[1]), 0, (struct sockaddr *)&server_address, sizeof(server_address));

    printf("Connected! Commands: /list, /msg <name> <text>, /stop\n");

    pthread_t receive_thread, send_thread;
    pthread_create(&receive_thread, NULL, receive_handler, NULL);
    pthread_create(&send_thread, NULL, send_handler, NULL);

    pthread_join(send_thread, NULL);
    close(sock);
    return 0;
}