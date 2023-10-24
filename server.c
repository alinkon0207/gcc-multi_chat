#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "define.h"
#include "tag.h"
#include "encDec.h"
#include "client_list.h"


int clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


void notifyAllClients()
{
    struct node * temp1 = head;

    if (temp1 != NULL && temp1->next == NULL)
        return;

    while (temp1 != NULL)
    {
        char temp_str[1000];
        struct node * temp2 = head;

        sprintf(temp_str, "<%s>", TAG_LOGIN_LIST);
        while (temp2 != NULL)
        {
            if (temp2 != temp1) // Omit self
                sprintf(temp_str + strlen(temp_str), "%s\n" , temp2->name);
            temp2 = temp2->next;
        }
        sprintf(temp_str + strlen(temp_str), "</%s>", TAG_LOGIN_LIST);

        send_with_crc(temp1->id, temp_str, strlen(temp_str), 0);

        temp1 = temp1->next;
    }
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    int to_client_socket = -1;
    char buffer[BUFFER_SIZE];

    // Receive and broadcast messages
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        if (recv_with_crc(client_socket, buffer, sizeof(buffer), 0) <= 0)
            break;

        xmlNode* root = parseXML(buffer);

        // Print the XML tree
        if (root != NULL)
        {
            if (strcmp(root->tag, TAG_REQUEST) == 0)
            {
                xmlNode* currentNode = root->children;
                if (currentNode != NULL)
                {
                    if (strcmp(currentNode->tag, TAG_LOGIN) == 0)
                    {
                        currentNode = currentNode->children;
                        char tempUsername[128];
                        char tempPassword[128];
                        char svrMsg[512];

                        while (currentNode != NULL)
                        {
                            if (strcmp(currentNode->tag, TAG_USERNAME) == 0)
                                strcpy(tempUsername, currentNode->content);    
                            else if(strcmp(currentNode->tag, TAG_PASSWORD) == 0)
                                strcpy(tempPassword, currentNode->content);
                            else
                            {
                                sprintf(svrMsg, "<%s>Undefined LogIn Type.</%s>", TAG_INFO, TAG_INFO);
                                send_with_crc(client_socket, svrMsg, strlen(svrMsg), 0);
                            }
                            currentNode = currentNode->next;
                        }

                        pthread_mutex_lock(&clients_mutex);
                        insertAtEnd(client_socket, tempUsername, tempPassword);
                        notifyAllClients();
                        pthread_mutex_unlock(&clients_mutex);
                        // printList();
                    }
                    else if(strcmp(currentNode->content, "disconnect") == 0)
                    {
                        close(client_socket);
                        pthread_mutex_lock(&clients_mutex);
                        deleteNode(client_socket);
                        notifyAllClients();
                        pthread_mutex_unlock(&clients_mutex);
                        // printList();
                    }
                }
            }
            else if (strcmp(root->tag, TAG_MSG) == 0)
            {
                xmlNode* currentNode = root->children;
                char from[128];
                char to[128];
                char svrMsg[BUFFER_SIZE];

                while (currentNode != NULL)
                {
                    if (strcmp(currentNode->tag, TAG_FROM) == 0)
                        sprintf(from, "%s", currentNode->content);    
                    else if (strcmp(currentNode->tag, TAG_TO) == 0)
                    {
                        sprintf(to, "%s", currentNode->content);
                        // printf("%s\n", currentNode->content);
                        to_client_socket = getSockClientFromName(currentNode->content);
                        // printf("%d\n", to_client_socket);
                    }
                    else if (strcmp(currentNode->tag, TAG_BODY) == 0)
                        sprintf(svrMsg, "<%s><%s>%s</%s><%s>%s</%s></%s>", 
                            TAG_MSG, 
                                TAG_FROM, from, TAG_FROM, 
                                TAG_BODY, currentNode->content, TAG_BODY, 
                            TAG_MSG);
                    
                    currentNode = currentNode->next;
                }

                if (to_client_socket == -1)
                {
                    sprintf(svrMsg, "<%s>'%s' does not exist now.</%s>", TAG_INFO, to, TAG_INFO);
                    send_with_crc(client_socket, svrMsg, strlen(svrMsg), 0);
                    break;
                }
                else
                    send_with_crc(to_client_socket, svrMsg, strlen(svrMsg), 0);
                 
                // printf("%s\n", svrMsg);
            }
        }
        
        pthread_mutex_unlock(&clients_mutex);
    }

    // Client disconnected
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == client_socket)
        {
            clients[i] = -1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    pthread_t tid;
    int port;

    if (argc == 1)
        port = PORT;
    else
        port = atoi(argv[1]);

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = -1;

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listening failed");
        return 1;
    }

    printf("Server started. Listening for connections...\n");

    while (1)
    {
        socklen_t client_len = sizeof(client_addr);

        // Accept new client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            perror("Accepting connection failed");
            return 1;
        }

        // Add client to the array
        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == -1)
            {
                clients[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        // Create a thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, &clients[i]) != 0)
        {
            perror("Thread creation failed");
            return 1;
        }
    }

    close(server_socket);
    return 0;
}
