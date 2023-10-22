#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tag.h"
#include "client_list.h"


#define MAX_CLIENTS 10
#define BUFFER_SIZE 50001
#define PORT        5000


int clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void notifyAllClients(){
    struct node * temp1 = head;
    struct node * temp2 = head;
    while(temp1 != NULL){
        char temp_str[256];
        while(temp2 != NULL){
            sprintf(temp_str,"<%s>%s</%s>" , TAG_LOGIN_LIST, temp2->name, TAG_LOGIN_LIST);
            send(temp1->id, temp_str, strlen(temp_str), 0);
            temp2 = temp2->next;
        }
        temp1 = temp1->next;
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    int to_client_socket = -1;
    char buffer[BUFFER_SIZE];

    // Receive and broadcast messages
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) {
            break;
        }

        xmlNode* root = parseXML(buffer);

        // Print the XML tree
        if (root != NULL) {
            if(strcmp(root->tag, TAG_REQUEST) == 0){
                xmlNode* currentNode = root->children;
                if(currentNode != NULL){
                    if(strcmp(currentNode->tag, TAG_LOGIN)==0){
                        currentNode = currentNode->children;
                        char tempPassword[128];
                        char tempName[128];
                        char svrMsg[512];
                        while (currentNode != NULL) {
                            if(strcmp(currentNode->tag, TAG_NAME)==0){
                                strcpy(tempName, currentNode->content);    
                            }else if(strcmp(currentNode->tag, TAG_PASSWORD)==0){
                                strcpy(tempPassword, currentNode->content);
                            }else {
                                sprintf(svrMsg, "<%s>Undefined LogIn Type.</%s>", TAG_INFO, TAG_INFO);
                                send(client_socket, svrMsg, strlen(svrMsg), 0);
                            }
                            currentNode = currentNode->next;
                        }
                        pthread_mutex_lock(&clients_mutex);
                        insertAtEnd(client_socket, tempName, tempPassword);
                        notifyAllClients();
                        pthread_mutex_unlock(&clients_mutex);
                        printList();
                    } else if(strcmp(currentNode->content, "disconnect") == 0){
                        close(client_socket);
                        pthread_mutex_lock(&clients_mutex);
                        deleteNode(client_socket);
                        notifyAllClients();
                        pthread_mutex_unlock(&clients_mutex);
                        printList();
                    }else{

                    }
                }
            }else if(strcmp(root->tag, TAG_MSG) == 0) {
                xmlNode* currentNode = root->children;
                char fromWhom[128];
                char svrMsg[BUFFER_SIZE];
                while (currentNode != NULL) {
                    if(strcmp(currentNode->tag, TAG_FROM)==0){
                        sprintf(fromWhom, "%s", currentNode->content);    
                    }else if(strcmp(currentNode->tag, TAG_TO)==0){
                        printf("%s\n", currentNode->content);
                        to_client_socket = getSockClientFromName(currentNode->content);
                        printf("%d\n", to_client_socket);
                    }else if(strcmp(currentNode->tag, TAG_BODY)==0){
                        sprintf(svrMsg, "msg from %s : %s", fromWhom, currentNode->content);    
                    }
                    currentNode = currentNode->next;
                }
                if(to_client_socket == -1){
                    sprintf(svrMsg, "<%s>%s does not exist now.</%s>", TAG_INFO, fromWhom, TAG_INFO);
                    send(client_socket, svrMsg, strlen(svrMsg), 0);
                    break;
                }else{
                    send(to_client_socket, svrMsg, strlen(svrMsg), 0);
                }    
                printf("%s\n", svrMsg);
            }else {

            }
        }
        
        pthread_mutex_unlock(&clients_mutex);
    }

    // Client disconnected
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == client_socket) {
            clients[i] = -1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listening failed");
        return 1;
    }

    printf("Server started. Listening for connections...\n");

    while (1) {
        socklen_t client_len = sizeof(client_addr);

        // Accept new client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accepting connection failed");
            return 1;
        }

        // Add client to the array
        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == -1) {
                clients[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        // Create a thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, &clients[i]) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    close(server_socket);
    return 0;
}
