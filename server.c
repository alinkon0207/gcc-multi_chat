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


#define MAX_CLIENTS 10
#define BUFFER_SIZE 50001
#define PORT        5000

//-----------------Lined lsit to manage client list-------------------------------------------
struct node{
    int id;
    char name[128];
    char password[128];
    struct node * next;
};

struct node * head = NULL;
struct ndoe * current = NULL;

void insertAtBegin(int _id, char * _name, char * password){
    // create a link
    struct node * lk = (struct node*)malloc(sizeof(struct node));
    memset(lk, 0, sizeof(struct node));
    // fill data to like's data field
    lk->id = _id;
    strcpy(lk->name, _name);
    strcpy(lk->password, password);
    // point it to old first node
    lk->next = head;
    // point first to new first node
    head = lk;
}

void insertAtEnd(int _id, char * _name, char * password){
    // create a link
    struct node * lk = (struct node*)malloc(sizeof(struct node));
    memset(lk, 0, sizeof(struct node));
    // fill data to like's data field
    lk->id = _id;
    strcpy(lk->name, _name);
    strcpy(lk->password, password);
    // create a temp link
    struct node * temp_link = head;
    // point it to old first node
    while(temp_link->next != NULL)
        temp_link = temp_link->next;
    // point first to new first node
    temp_link->next = lk;
}

void insertAfterNode(struct node * list, int _id, char * _name, char * password){
    // create a link
    struct node * lk = (struct node*)malloc(sizeof(struct node));
    memset(lk, 0, sizeof(struct node));
    // fill data to like's data field
    lk->id = _id;
    strcpy(lk->name, _name);
    strcpy(lk->password, password);
    lk->next = list->next;
    list->next = lk;
}

void deleteAtBegin(){
    struct node * ptr = head;
    if(ptr != NULL)
        free(ptr);
    head = head->next;
}
void deleteAtEnd(){
    struct node * linkedlist = head;
    while(linkedlist->next->next != NULL)
        linkedlist = linkedlist->next;
    linkedlist->next = NULL;
}
void deleteNode(int _id){
    struct node * temp = head, *prev;
    if(temp != NULL && temp->id == _id){
        head = temp->next;
        return;
    }

    while(temp != NULL && temp->id != _id){
        prev = temp;
        temp = temp->next;
    }

    if(temp == NULL) return;

    prev->next = temp->next;
}

int getListItemCount(){
    int count = 0;
    struct node * temp = head;
    while(temp != NULL){
        count ++;
        temp = temp->next;
    }
    return count;
}

int getSockClientFromName(char * _name) {
    struct node * currentNode = head;
    printf("getSockClientFromName --> %s\n", _name);
    while (currentNode != NULL) {
        if (strcmp(currentNode->name, _name) == 0) {
            return currentNode->id;
        }
        currentNode = currentNode->next;
    }
    return -1; // Node with given data not found
}

void printList(){
    struct node *p = head;
    printf("\nClient List");// 
    printf("\n-----------------------------------------------------\n");
    //start from the beginning
    while(p != NULL) {
        printf(" id: %d | name: %s |  password: %s ", p->id, p->name, p->password);
        printf("\n----------------------------------------------------------\n");
        p = p->next;
    }
    printf("\n----------------------------------------------------------\n");
}

//---------------------------------------------parse message---------------------------------

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

//---------------------------------------------main function----------------------------------


typedef struct {
    int socket;
    char username[20];
} Client;

Client clients[MAX_CLIENTS];
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

        pthread_mutex_lock(&clients_mutex);
        if (1) {
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
                            insertAtBegin(client_socket, tempName, tempPassword);
                            printList();
                            notifyAllClients();
                        } else if(strcmp(currentNode->content, "disconnect") == 0){
                            close(client_socket);
                            deleteNode(client_socket);
                            printList();
                            notifyAllClients();
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
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    // Client disconnected
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == client_socket) {
            clients[i].socket = -1;
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
        clients[i].socket = -1;
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
            if (clients[i].socket == -1) {
                clients[i].socket = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        // Create a thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, &clients[i].socket) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    close(server_socket);
    return 0;
}
