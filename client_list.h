#ifndef __CLIENT_LIST_H__
#define __CLIENT_LIST_H__

//-----------------Lined lsit to manage client list-------------------------------------------
struct node{
    int id;
    char name[128];
    char password[128];
    struct node * next;
};

extern struct node * head;
extern struct node * current;


void insertAtEnd(int _id, char * _name, char * password);
void deleteNode(int _id);

int getSockClientFromName(char * _name);
void printList();

#endif  // __CLIENT_LIST_H__
