#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tag.h"

// Function to create a new node
xmlNode* createNode(const char* tag, const char* content)
{
    xmlNode* newNode = (xmlNode*)malloc(sizeof(xmlNode));
    newNode->tag = strdup(tag);
    newNode->content = strdup(content);
    newNode->children = NULL;
    newNode->next = NULL;
    newNode->parent = NULL;
    return newNode;
}

// Function to insert a child node
void insertChild(xmlNode* parent, xmlNode* child)
{
    if (parent->children == NULL)
    {
        parent->children = child;
        child->parent = parent;
    }
    else
    {
        xmlNode* lastChild = parent->children;

        while (lastChild->next != NULL)
            lastChild = lastChild->next;
        lastChild->next = child;
        child->parent = parent;
    }
}

// Function to parse the XML string and build the tree
xmlNode* parseXML(const char* xmlString)
{
    xmlNode* root = NULL;
    xmlNode* currentNode = NULL;

    int xmlLength = strlen(xmlString);
    int i = 0;

    while (i < xmlLength)
    {
        if (xmlString[i] == '<')
        {
            // Find the end of the tag
            int end = i + 1;
            while (end < xmlLength && xmlString[end] != '>')
                end++;

            if (end < xmlLength)
            {
                if (xmlString[i + 1] == '/')
                {
                    // Closing tag
                    if (currentNode != NULL)
                        currentNode = currentNode->parent;
                }
                else
                {
                    // Opening tag
                    int tagNameLength = end - i - 1;
                    char tagName[tagNameLength + 1];
                    strncpy(tagName, &xmlString[i + 1], tagNameLength);
                    tagName[tagNameLength] = '\0';

                    xmlNode* newNode = createNode(tagName, "");
                    if (root == NULL)
                    {
                        root = newNode;
                        currentNode = root;
                    }
                    else
                    {
                        insertChild(currentNode, newNode);
                        currentNode = newNode;
                    }
                }
            }

            i = end + 1;
        }
        else
        {
            // Content
            int contentEnd = i;
            while (contentEnd < xmlLength && xmlString[contentEnd] != '<')
                contentEnd++;

            int contentLength = contentEnd - i;
            char content[contentLength + 1];
            strncpy(content, &xmlString[i], contentLength);
            content[contentLength] = '\0';

            if (currentNode != NULL)
                currentNode->content = strdup(content);

            i = contentEnd;
        }
    }

    return root;
}

void extract_content(char* xml_string, char* tag_name, char* content)
{
    char* start_tag = NULL;
    char* end_tag = NULL;
    char* start_content = NULL;
    char* end_content = NULL;
    int tag_len = strlen(tag_name);
    char* ptr = xml_string;

    while (*ptr != '\0')
    {
        if (*ptr == '<')
        {
            if (start_tag == NULL && strncmp(ptr+1, tag_name, tag_len) == 0)
            {
                start_tag = ptr;
                start_content = ptr + tag_len + 2; // skip tag and opening bracket
                break;
            }
        }

        ptr++;
    }

    ptr = xml_string + strlen(xml_string) - 2;
    while (ptr >= start_content)
    {
        if (*ptr == '<' && *(ptr + 1) == '/')
        {
            if (end_tag == NULL && strncmp(ptr + 2, tag_name, tag_len) == 0)
            {
                end_content = ptr;
                end_tag = ptr + tag_len + 3;
                break;
            }
        }

        ptr--;
    }
    
    if (start_tag == NULL || end_tag == NULL || start_content == NULL || end_content == NULL)
    {
        printf("Error: Could not find tag '%s'\n", tag_name);
        return;
    }
    
    strncpy(content, start_content, end_content - start_content);
    content[end_content - start_content] = '\0';
}
