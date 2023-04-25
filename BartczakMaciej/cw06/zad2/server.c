#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mqueue.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include "common.h"

#define MAX_CLIENTS 20

mqd_t clientQueueDescriptors[MAX_CLIENTS];
mqd_t serverQueueDescriptor;

void stopServer();

void handle_INIT(MessageBuffer *);

void handle_STOP(MessageBuffer *);

void handle_LIST(MessageBuffer *);

void handle_2ALL(MessageBuffer *);

void handle_2ONE(MessageBuffer *);

void logMessage(MessageBuffer *messageBuffer, bool);

int main(int argc, char *argv[]) {
    signal(SIGINT, stopServer);
    signal(SIGTERM, stopServer);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clientQueueDescriptors[i] = -1;
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(MessageBuffer);
    attr.mq_curmsgs = 0;

    serverQueueDescriptor = mq_open(SERVER_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);

    MessageBuffer messageBuffer;
    while (1) {
        if (mq_receive(serverQueueDescriptor, (char *) &messageBuffer, sizeof(MessageBuffer), NULL) != -1) {
            logMessage(&messageBuffer, true);

            RequestType rType = messageBuffer.type;
            switch (rType) {
                case COMMON_STOP:
                    handle_STOP(&messageBuffer);
                    break;
                case COMMON_INIT:
                    handle_INIT(&messageBuffer);
                    break;
                case SERVER_LIST:
                    handle_LIST(&messageBuffer);
                    break;
                case SERVER_TO_ALL:
                    handle_2ALL(&messageBuffer);
                    break;
                case SERVER_TO_ONE:
                    handle_2ONE(&messageBuffer);
                    break;
                default:
                    printf("Invalid request received (%d)\n", rType);
                    break;
            }
        }
    }
}

void stopServer() {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));
    messageBuffer->type = COMMON_STOP;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientQueueDescriptors[i] == -1) {
            continue;
        }

        logMessage(messageBuffer, false);
        mq_send(clientQueueDescriptors[i], (const char *) messageBuffer, sizeof(MessageBuffer), 0);
    }
    free(messageBuffer);

    mq_close(serverQueueDescriptor);
    mq_unlink(SERVER_QUEUE_NAME);
    printf("EXITING...\n");
    exit(EXIT_SUCCESS);
}

void handle_INIT(MessageBuffer *messageBuffer) {
    int newClientID;
    bool initPossible = false;
    for (newClientID = 0; newClientID < MAX_CLIENTS; ++newClientID) {
        if (clientQueueDescriptors[newClientID] == -1) {
            initPossible = true;
            break;
        }
    }

    if (initPossible) {
        printf("INIT: clientID=%d\n", newClientID);
        messageBuffer->clientID = newClientID;
        clientQueueDescriptors[newClientID] = mq_open(messageBuffer->data, O_WRONLY | O_NONBLOCK);
    } else {
        printf("INIT: client limit reached\n");
        messageBuffer->clientID = -1;
    }

    logMessage(messageBuffer, false);
    mq_send(clientQueueDescriptors[newClientID], (const char *) messageBuffer, sizeof(MessageBuffer), 0);
}

void handle_STOP(MessageBuffer *messageBuffer) {
    int clientID = messageBuffer->clientID;
    printf("STOP: clientID=%d\n", clientID);
    mq_close(clientQueueDescriptors[clientID]);
    clientQueueDescriptors[clientID] = -1;
}

void handle_LIST(MessageBuffer *messageBuffer) {
    int clientID = messageBuffer->clientID;
    char *list = messageBuffer->data;
    list[0] = '\0';

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientQueueDescriptors[i] != -1) {
            char buf[10];
            sprintf(buf, "%d ", i);
            strcat(list, buf);
        }
    }

    logMessage(messageBuffer, false);
    mq_send(clientQueueDescriptors[clientID], (const char *) messageBuffer, sizeof(MessageBuffer), 0);
}

void handle_2ALL(MessageBuffer *messageBuffer) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientQueueDescriptors[i] != -1 && i != messageBuffer->clientID) {
            logMessage(messageBuffer, false);
            mq_send(clientQueueDescriptors[i], (const char *) messageBuffer, sizeof(MessageBuffer), 0);
        }
    }
}

void handle_2ONE(MessageBuffer *messageBuffer) {
    int destination = messageBuffer->destinationID;
    if (clientQueueDescriptors[destination] != -1) {
        logMessage(messageBuffer, false);
        mq_send(clientQueueDescriptors[destination], (const char *) messageBuffer, sizeof(MessageBuffer), 0);
    }
}

void logMessage(MessageBuffer *messageBuffer, bool incoming) {
    if (incoming) {
        printf("IN:  clientID=%d, type=%ld\n", messageBuffer->clientID, messageBuffer->type);
    } else {
        printf("OUT: clientID=%d, type=%ld\n", messageBuffer->clientID, messageBuffer->type);
    }
}

