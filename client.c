#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "define.h"
#include "tag.h"
#include "crc.h"
#include "encDec.h"


void *receive_messages(void *arg)
{
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE * 2];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        if (recv_with_crc(client_socket, buffer, sizeof(buffer), 0) <= 0)
            break;

        char startTag[128];
        char from[128];
        char content[128];
        sprintf(startTag, "<%s>", TAG_LOGIN_LIST);
        if (strncmp(buffer, startTag, strlen(startTag)) == 0)
        {
            extract_content(buffer, TAG_LOGIN_LIST, content);
            printf("\nClient List");
            printf("\n-----------------------------------------------------\n");
            printf("%s", content);
            printf("-----------------------------------------------------\n");
            continue;
        }

        sprintf(startTag, "<%s>", TAG_MSG);
        if (strncmp(buffer, startTag, strlen(startTag)) == 0)
        {
            extract_content(buffer, TAG_FROM, from);
            extract_content(buffer, TAG_BODY, content);
            // printf("content: %s\n", content);

            char msg[BUFFER_SIZE];

            hamming_decode(content, msg, strlen(content));
            printf("msg from %s: %s\n", from, msg);
            continue;
        }
        else
            printf("%s\n", buffer);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t tid;
    char ip[20] = "";
    int port;

    if (argc < 2)
        strcpy(ip, "127.0.0.1");
    else
        strcpy(ip, argv[1]);
    if (argc < 3)
        port = PORT;
    else
        port = atoi(argv[2]);

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        return 1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to the server\n");

    // Create a thread to receive messages from the server
    if (pthread_create(&tid, NULL, receive_messages, &client_socket) != 0)
    {
        perror("Thread creation failed");
        return 1;
    }

    // Send messages to the server
    char message[BUFFER_SIZE];
    int login = 0;
    char answer;
    char username[64];
    char password[64];

    while (1)
    {
        if (login == 0)
        {
            printf("LOGIN? [y/n] ");
            answer = getchar();
            getchar();
            if (answer == 'y')
            {
                int login_succeed = 0;

                while (login_succeed == 0)
                {
                    printf("You choose LOGIN option...\n");
                    
                    printf("username: ");
                    if (fgets(username, sizeof(username), stdin))
                    {
                        username[strcspn(username, "\n")] = '\0';
                        
                        printf("password: ");
                        if (fgets(password, sizeof(password), stdin))
                        {
                            password[strcspn(password, "\n")] = '\0';

                            sprintf(message, "<%s><%s><%s>%s</%s><%s>%s</%s></%s></%s>\n", 
                                TAG_REQUEST, TAG_LOGIN, TAG_USERNAME, username, TAG_USERNAME, TAG_PASSWORD, password, TAG_PASSWORD, TAG_LOGIN, TAG_REQUEST);
                            send_with_crc(client_socket, message, strlen(message), 0);

                            login_succeed = 1;
                        }
                    }
                }

                login = 1;
            }
            else if (answer == 'n')
                continue;
            else
            {
                printf("input correct option character\n");
                break;
            }
        }
        else
        {
            char option;

            printf("Please select one of the following options\n");
            printf("1: MSG\n");
            printf("2: DISCONNECT\n");
            answer = getchar();
            getchar();
            if (answer == '1')
            {
                char to_name[100];
                char temp_msg[BUFFER_SIZE];
                uint8_t enc_msg[BUFFER_SIZE * 2];
                char send_msg[BUFFER_SIZE * 2 + 1000];

                printf("to: ");
                if (fgets(to_name, sizeof(to_name), stdin) != NULL)
                {
                    to_name[strcspn(to_name, "\n")] = '\0';
                    
                    printf("msg: ");
                    if (fgets(temp_msg, sizeof(temp_msg), stdin) != NULL)
                    {
                        temp_msg[strcspn(temp_msg, "\n")] = '\0';

                        int enc_len = hamming_encode(temp_msg, enc_msg, strlen(temp_msg));
                        printf("temp_msg = %s, enc_len = %d, enc_msg = %s\n", temp_msg, enc_len, enc_msg);

                        sprintf(send_msg, "<%s><%s>%s</%s><%s>%s</%s><%s>%s</%s></%s>", 
                            TAG_MSG, 
                                TAG_FROM, username, TAG_FROM, 
                                TAG_TO, to_name, TAG_TO, 
                                TAG_BODY, /* temp_msg */ enc_msg, TAG_BODY, 
                            TAG_MSG);
                        send_with_crc(client_socket, send_msg, strlen(send_msg), 0);
                    }
                }
            }
            else if (answer == '2')
            {
                printf("Disconnect? [y/n] ");
                answer = getchar();
                getchar();
                if (answer == 'y')
                {
                    sprintf(message, "<%s>disconnect<%s>", TAG_REQUEST, TAG_REQUEST);
                    send_with_crc(client_socket, message, strlen(message), 0);
                    login = 0;
                }
            }
            else
            {
                printf("input correct option\n");
                break;
            }
        }
    }

    close(client_socket);
    return 0;
}
