#include "client_list.h"

struct node * head = NULL;
struct ndoe * current = NULL;


void insertAtBegin(int _id, char * _name, char * password)
{
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

void insertAtEnd(int _id, char * _name, char * password)
{
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

void insertAfterNode(struct node * list, int _id, char * _name, char * password)
{
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

// void deleteAtBegin(){
//     struct node * ptr = head;
//     if(ptr != NULL)
//         free(ptr);
//     head = head->next;
// }

// void deleteAtEnd(){
//     struct node * linkedlist = head;
//     while(linkedlist->next->next != NULL)
//         linkedlist = linkedlist->next;
//     linkedlist->next = NULL;
// }

void deleteNode(int _id)
{
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

// int getListItemCount(){
//     int count = 0;
//     struct node * temp = head;
//     while(temp != NULL){
//         count ++;
//         temp = temp->next;
//     }
//     return count;
// }

int getSockClientFromName(char * _name)
{
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

void printList()
{
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
