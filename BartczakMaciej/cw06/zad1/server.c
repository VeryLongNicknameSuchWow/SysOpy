#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>
#include "common.h"

#define MAX_CLIENTS 20

key_t clientQueueKeys[MAX_CLIENTS];
int serverQueueID;

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
        clientQueueKeys[i] = -1;
    }

    key_t serverQueueKey = ftok(HOME_PATH, SERVER_ID);
    serverQueueID = msgget(serverQueueKey, IPC_CREAT | 0666);

    MessageBuffer messageBuffer;
    while (true) {
        msgrcv(serverQueueID, &messageBuffer, sizeof(MessageBuffer), -6, 0);
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

void stopServer() {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));
    messageBuffer->type = COMMON_STOP;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientQueueKeys[i] == -1) {
            continue;
        }

        int destinationQueueID = msgget(clientQueueKeys[i], 0);
        logMessage(messageBuffer, false);
        msgsnd(destinationQueueID, messageBuffer, sizeof(MessageBuffer), 0);
    }
    free(messageBuffer);

    msgctl(serverQueueID, IPC_RMID, NULL);
    printf("EXITING...\n");
    exit(EXIT_SUCCESS);
}

void handle_INIT(MessageBuffer *messageBuffer) {
    int newClientID;
    bool initPossible = false;
    for (newClientID = 0; newClientID < MAX_CLIENTS; ++newClientID) {
        if (clientQueueKeys[newClientID] == -1) {
            initPossible = true;
            break;
        }
    }

    if (initPossible) {
        printf("INIT: clientID=%d\n", newClientID);
        messageBuffer->clientID = newClientID;
        clientQueueKeys[newClientID] = messageBuffer->queueKey;
    } else {
        fprintf(stderr, "INIT: client limit reached\n");
        messageBuffer->clientID = -1;
    }

    int clientQueueID = msgget(messageBuffer->queueKey, 0);
    logMessage(messageBuffer, false);
    msgsnd(clientQueueID, messageBuffer, sizeof(MessageBuffer), 0);
}

void handle_STOP(MessageBuffer *messageBuffer) {
    int clientID = messageBuffer->clientID;
    printf("STOP: clientID=%d\n", clientID);

    clientQueueKeys[clientID] = -1;
}

void handle_LIST(MessageBuffer *messageBuffer) {
    int clientID = messageBuffer->clientID;
    printf("LIST: clientID=%d\n", clientID);

    strcpy(messageBuffer->data, "");
    char buffer[128];
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientQueueKeys[i] == -1) {
            continue;
        }
        sprintf(buffer, "%d", i);
        strcat(messageBuffer->data, buffer);
        if (i < MAX_CLIENTS - 1) {
            strcat(messageBuffer->data, ",");
        }
    }
    messageBuffer->data[strlen(messageBuffer->data) - 1] = '\0';

    int clientQueueID = msgget(clientQueueKeys[clientID], 0);
    logMessage(messageBuffer, false);
    msgsnd(clientQueueID, messageBuffer, sizeof(MessageBuffer), 0);
}

void handle_2ALL(MessageBuffer *messageBuffer) {
    int clientID = messageBuffer->clientID;
    printf("2ALL: clientID=%d\n", clientID);

    messageBuffer->type = CLIENT_MESSAGE;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (i == clientID || clientQueueKeys[i] == -1) {
            continue;
        }

        int destinationQueueID = msgget(clientQueueKeys[i], 0);
        logMessage(messageBuffer, false);
        msgsnd(destinationQueueID, messageBuffer, sizeof(MessageBuffer), 0);
    }
}

void handle_2ONE(MessageBuffer *messageBuffer) {
    int clientID = messageBuffer->clientID;
    printf("2ONE: clientID=%d\n", clientID);

    messageBuffer->type = CLIENT_MESSAGE;
    int destinationQueueID = msgget(clientQueueKeys[messageBuffer->destinationID], 0);
    logMessage(messageBuffer, false);
    msgsnd(destinationQueueID, messageBuffer, sizeof(MessageBuffer), 0);
}

void logMessage(MessageBuffer *messageBuffer, bool incoming) {
    FILE *logFile = fopen("server.log", "a+");
    if (incoming) {
        fprintf(logFile, "INCOMING MESSAGE\n");
    } else {
        fprintf(logFile, "OUTGOING MESSAGE\n");
    }
    fprintf(logFile, "epoch: %ld\n", time(NULL));
    fprintf(logFile, "type: %ld\n", messageBuffer->type);
    fprintf(logFile, "key: %d\n", messageBuffer->queueKey);
    fprintf(logFile, "cid: %d\n", messageBuffer->clientID);
    fprintf(logFile, "did: %d\n", messageBuffer->destinationID);
    fprintf(logFile, "data: \"%s\"\n", messageBuffer->data);
    fclose(logFile);
}
