#ifndef SYSOPY_COMMON_H
#define SYSOPY_COMMON_H

#define HOME_PATH getenv("HOME")
#define SERVER_ID 1
#define MESSAGE_DATA_SIZE 1024

typedef struct MessageBuffer {
    long type;
    key_t queueKey;
    int clientID;
    int destinationID;
    char data[MESSAGE_DATA_SIZE];
} MessageBuffer;

typedef enum RequestType {
    COMMON_STOP = 1,
    COMMON_INIT = 2,
    SERVER_LIST = 3,
    SERVER_TO_ALL = 4,
    SERVER_TO_ONE = 5,
    CLIENT_MESSAGE = 6,
} RequestType;

#endif //SYSOPY_COMMON_H
