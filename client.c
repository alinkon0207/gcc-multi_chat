#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tag.h"


#define BUFFER_SIZE 50001
#define PORT        5000


void *receive_messages(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0) {
            break;
        }
       char startTag[128];
       char content[128];
       sprintf(startTag, "<%s>", TAG_LOGIN_LIST);
       int start = strstr(buffer, startTag) - buffer;
       if(start == 0){
            extract_content(buffer, TAG_LOGIN_LIST, content);
            printf("%s\n", content);
       }else{
            printf("%s\n", buffer);
       }
    }
    pthread_exit(NULL);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t tid;

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0) {
        perror("Invalid address");
        return 1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to the server\n");

    // Create a thread to receive messages from the server
    if (pthread_create(&tid, NULL, receive_messages, &client_socket) != 0) {
        perror("Thread creation failed");
        return 1;
    }

    // Send messages to the server
    char message[BUFFER_SIZE];
    int login = -1;
    char username[64];
    char password[64];
    while (1)
    {
        if(login == -1)
        {
            printf("LOGIN ? [y/n]\n");
            char user_answer[10];
           
            fgets(user_answer, sizeof(user_answer), stdin);
            user_answer[strcspn(user_answer, "\n")] = '\0';
            if (!strcmp(user_answer, "y")){
                int login_succeed = -1;
                while(login_succeed == -1){
                    printf("You choose LOGIN option...\n");
                    
                    printf("\nusername: ");
                    if(fgets(username, sizeof(username), stdin)){
                        printf("\npassword: ");
                        username[strcspn(username, "\n")] = '\0';
                        if(fgets(password, sizeof(password), stdin)){
                            password[strcspn(password, "\n")] = '\0';     
                            login_succeed = 1;
                            sprintf(message, "<%s><%s><%s>%s</%s><%s>%s</%s></%s></%s>\n", 
                                TAG_REQUEST, TAG_LOGIN, TAG_NAME, username, TAG_NAME, TAG_PASSWORD, password, TAG_PASSWORD, TAG_LOGIN, TAG_REQUEST);
                            send(client_socket, message, strlen(message), 0);
                        }
                    }
                }
                login = 1;
            }else if (!strcmp(user_answer, "n")){
                continue;
            }else{
                printf("input correct option character\n");
                break;
            }
        }
        else{
            int option = -1;
            printf("1: MSG\n");
            printf("2: DISCONNECT\n");
            scanf("%d", &option);
            if(option == 1){
                char to_name[128];
                char temp_msg[49000];
                char send_msg[BUFFER_SIZE];
                fgets(to_name, sizeof(to_name), stdin);
                printf("\nto: ");
                if(fgets(to_name, sizeof(to_name), stdin) != NULL){
                    to_name[strcspn(to_name, "\n")] = '\0';     
                    printf("\nmsg: ");    
                    if(fgets(temp_msg, sizeof(temp_msg), stdin) != NULL){
                        temp_msg[strcspn(temp_msg, "\n")] = '\0';     
                        sprintf(send_msg, "<%s><%s>%s</%s><%s>%s</%s><%s>%s</%s></%s>", 
                            TAG_MSG, TAG_FROM, username, TAG_FROM, TAG_TO, to_name, TAG_TO, TAG_BODY, temp_msg, TAG_BODY, TAG_MSG);
                        send(client_socket, send_msg, strlen(send_msg), 0);
                    }
                }
            }else if(option == 2){
                printf("disconnect? [y/n]\n");
                char inChar;
                scanf("%c", &inChar);
                printf("here\n");
                if(inChar == 'y'){
                    sprintf(message, "<%s>disconnect<%s>", TAG_REQUEST, TAG_REQUEST);
                    send(client_socket, message, strlen(message), 0);
                    break;
                }
            }else{
                break;
            }
        }
    }

    close(client_socket);
    return 0;
}
