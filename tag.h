#ifndef __TAG_H__
#define __TAG_H__

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

// Structure to represent a node in the XML tree
typedef struct xmlNode {
    char* tag;
    char* content;
    struct xmlNode* children;
    struct xmlNode* next;
    struct xmlNode* parent;
} xmlNode;

xmlNode* parseXML(const char* xmlString);
void extract_content(char* xml_string, char* tag_name, char* content);

#endif  // __TAG_H__
