#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TAG_REQUEST     "REQUEST"
#define TAG_MSG         "MSG"
#define TAG_LOGIN       "LOGIN"
#define TAG_LOGIN_LIST  "LOGIN_LIST"
#define TAG_INFO        "INFO"
#define TAG_TO          "TO"
#define TAG_FROM        "FROM"
#define TAG_BODY        "BODY"
#define TAG_NAME        "NAME"
#define TAG_PASSWORD    "PASSWORD"

#define BUFFER_SIZE 50001
#define PORT        5000


// Structure to represent a node in the XML tree
typedef struct xmlNode {
    char* tag;
    char* content;
    struct xmlNode* children;
    struct xmlNode* next;
    struct xmlNode* parent;
} xmlNode;

// Function to create a new node
xmlNode* createNode(const char* tag, const char* content) {
    xmlNode* newNode = (xmlNode*)malloc(sizeof(xmlNode));
    newNode->tag = strdup(tag);
    newNode->content = strdup(content);
    newNode->children = NULL;
    newNode->next = NULL;
    newNode->parent = NULL;
    return newNode;
}

// Function to insert a child node
void insertChild(xmlNode* parent, xmlNode* child) {
    if (parent->children == NULL) {
        parent->children = child;
        child->parent = parent;
    } else {
        xmlNode* lastChild = parent->children;
        while (lastChild->next != NULL) {
            lastChild = lastChild->next;
        }
        lastChild->next = child;
        child->parent = parent;
    }
}

// Function to parse the XML string and build the tree
xmlNode* parseXML(const char* xmlString) {
    xmlNode* root = NULL;
    xmlNode* currentNode = NULL;

    int xmlLength = strlen(xmlString);
    int i = 0;

    while (i < xmlLength) {
        if (xmlString[i] == '<') {
            // Find the end of the tag
            int end = i + 1;
            while (end < xmlLength && xmlString[end] != '>') {
                end++;
            }

            if (end < xmlLength) {
                if (xmlString[i + 1] == '/') {
                    // Closing tag
                    if (currentNode != NULL) {
                        currentNode = currentNode->parent;
                    }
                } else {
                    // Opening tag
                    int tagNameLength = end - i - 1;
                    char tagName[tagNameLength + 1];
                    strncpy(tagName, &xmlString[i + 1], tagNameLength);
                    tagName[tagNameLength] = '\0';

                    xmlNode* newNode = createNode(tagName, "");
                    if (root == NULL) {
                        root = newNode;
                        currentNode = root;
                    } else {
                        insertChild(currentNode, newNode);
                        currentNode = newNode;
                    }
                }
            }

            i = end + 1;
        } else {
            // Content
            int contentEnd = i;
            while (contentEnd < xmlLength && xmlString[contentEnd] != '<') {
                contentEnd++;
            }

            int contentLength = contentEnd - i;
            char content[contentLength + 1];
            strncpy(content, &xmlString[i], contentLength);
            content[contentLength] = '\0';

            if (currentNode != NULL) {
                currentNode->content = strdup(content);
            }

            i = contentEnd;
        }
    }

    return root;
}

void extract_content(char* xml_string, char* tag_name, char* content) {
    char* start_tag = NULL;
    char* end_tag = NULL;
    char* start_content = NULL;
    char* end_content = NULL;
    int tag_len = strlen(tag_name);
    int content_len = 0;
    char* ptr = xml_string;
    
    while (*ptr != '\0') {
        if (*ptr == '<') {
            if (strncmp(ptr+1, tag_name, tag_len) == 0) {
                start_tag = ptr;
                start_content = ptr + tag_len + 2; // skip tag and opening bracket
            }
        } else if (*ptr == '>') {
            if (start_tag != NULL) {
                end_tag = ptr;
                end_content = ptr;
                break;
            }
        } else if (start_tag != NULL) {
            content_len++;
        }
        ptr++;
    }
    
    if (start_tag == NULL || end_tag == NULL || start_content == NULL || end_content == NULL) {
        printf("Error: Could not find tag '%s'\n", tag_name);
        return;
    }
    
    strncpy(content, start_content, content_len);
    content[content_len] = '\0';
}

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
    while (1) {
        if(login == -1){
            printf("LOGIN ? [y/n]\n");
            char user_answer;
           
            scanf("%c", &user_answer);
            if(user_answer == 'y'){
                int login_succeed = -1;
                while(login_succeed == -1){
                    printf("You choose LOGIN option...\n");
                    fgets(username, sizeof(username), stdin);
                    #if 0
                    fgets(username, sizeof(username), stdin);
                    printf("password:");
                    fgets(password, sizeof(password), stdin);
                    #endif
                    
                    printf("\nusername :");
                    if(fgets(username, sizeof(username), stdin)){
                        printf("\npassword: ");
                        username[strcspn(username, "\n")] = '\0';     
                        if(fgets(password, sizeof(password), stdin)){
                            password[strcspn(password, "\n")] = '\0';     
                            login_succeed = 1;
                            sprintf(message, "<%s><%s><%s>%s</%s><%s>%s</%s></%s></%s>\n", 
                                TAG_REQUEST, TAG_LOGIN, TAG_NAME, username, TAG_NAME, TAG_PASSWORD, password, TAG_PASSWORD, TAG_LOGIN, TAG_REQUEST);
                            printf("%s", message);
                            send(client_socket, message, strlen(message), 0);
                        }
                    }
                }
                login = 1;
            }else if(user_answer == 'n'){
                break;
            }else{
                printf("input correct option character\n");
            }
        }else{
            int option = -1;
            printf("1 : MSG\n");
            printf("2 : REQUEST\n");
            scanf("%d", &option);
            if(option == 1){
                char to_name[128];
                char temp_msg[49000];
                char send_msg[BUFFER_SIZE];
                fgets(to_name, sizeof(to_name), stdin);
                printf("\nto : ");
                if(fgets(to_name, sizeof(to_name), stdin) != NULL){
                    to_name[strcspn(to_name, "\n")] = '\0';     
                    printf("\nmsg : ");    
                    if(fgets(temp_msg, sizeof(temp_msg), stdin) != NULL){
                        temp_msg[strcspn(temp_msg, "\n")] = '\0';     
                        sprintf(send_msg, "<%s><%s>%s</%s><%s>%s</%s><%s>%s</%s></%s>", 
                            TAG_MSG, TAG_FROM, username, TAG_FROM, TAG_TO, to_name, TAG_TO, TAG_BODY, temp_msg, TAG_BODY, TAG_MSG);
                        printf("%s\n", send_msg);
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
