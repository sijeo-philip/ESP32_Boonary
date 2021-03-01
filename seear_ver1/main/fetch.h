#ifndef _FETCH_H_
#define _FETCH_H_

typedef struct 
{
    char *key;
    char *val;
}Header;

typedef enum
{
    Get, 
    Post
}HttpMethod;


struct DataParams
{
    void(*onGotData)(char* incomingBuffer, char* output);
    char* message;
    Header header[3];
    int headerCount;
    HttpMethod method;
    char* body;
    int status;
};

void data_transact(char* url, struct DataParams *dataParams);

#endif