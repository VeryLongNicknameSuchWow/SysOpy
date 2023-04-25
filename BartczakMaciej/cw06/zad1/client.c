#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "common.h"

#define INPUT_BUFFER_SIZE 1024

key_t clientQueueKey;
int clientQueueID;
int serverQueueID;
int clientID;

void action_INIT();

void action_LIST();

void action_2ONE(int destination, char *message);

void action_2ALL(char *message);

void action_STOP();

void handle_MESSAGE();

int main(int argc, char *argv[]) {
    signal(SIGINT, action_STOP);
    signal(SIGTERM, action_STOP);

    srand(time(NULL));
    clientQueueKey = ftok(HOME_PATH, rand() % 255 + 1);
    clientQueueID = msgget(clientQueueKey, IPC_CREAT | 0666);
    key_t serverQueueKey = ftok(HOME_PATH, SERVER_ID);
    serverQueueID = msgget(serverQueueKey, 0);
    action_INIT();

    if (clientID < 0) {
        fprintf(stderr, "Received invalid clientID, server may be at capacity\n");
        return EXIT_FAILURE;
    }

    char buffer[INPUT_BUFFER_SIZE];
    while (1) {
        handle_MESSAGE();

        printf(">");
        if (fgets(buffer, INPUT_BUFFER_SIZE, stdin) == NULL) {
            if (ferror(stdin)) {
                fprintf(stderr, "An error occurred while reading stdin (%s)\n", strerror(errno));
            }
            fprintf(stderr, "EOF reached on stdin\n");
            action_STOP();
            break;
        }

        char *delim = " \n";
        char *command = strtok(buffer, delim);
        if (command == NULL) {
            continue;
        }

        if (strcmp(command, "STOP") == 0) {
            action_STOP();
            return EXIT_SUCCESS;
        } else if (strcmp(command, "LIST") == 0) {
            action_LIST();
        } else if (strcmp(command, "2ALL") == 0) {
            char *message = strtok(NULL, delim);
            action_2ALL(message);
        } else if (strcmp(command, "2ONE") == 0) {
            char *destinationStr = strtok(NULL, delim);
            int destinationID = atoi(destinationStr);
            char *message = strtok(NULL, delim);
            action_2ONE(destinationID, message);
        } else {
            printf("Invalid command!\n");
        }
    }
}

void action_INIT() {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));

    messageBuffer->type = COMMON_INIT;
    messageBuffer->queueKey = clientQueueKey;

    msgsnd(serverQueueID, messageBuffer, sizeof(MessageBuffer), 0);
    msgrcv(clientQueueID, messageBuffer, sizeof(MessageBuffer), 0, 0);

    int newClientID = messageBuffer->clientID;
    free(messageBuffer);

    printf("INIT: received clientID=%d\n", newClientID);
    clientID = newClientID;
}

void action_LIST() {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));

    messageBuffer->type = SERVER_LIST;
    messageBuffer->clientID = clientID;

    msgsnd(serverQueueID, messageBuffer, sizeof(MessageBuffer), 0);
    msgrcv(clientQueueID, messageBuffer, sizeof(MessageBuffer), 0, 0);

    printf("LIST: %s\n", messageBuffer->data);
    free(messageBuffer);
}

void action_2ONE(int destination, char *message) {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));

    messageBuffer->type = SERVER_TO_ONE;
    messageBuffer->clientID = clientID;
    messageBuffer->destinationID = destination;
    strncpy(messageBuffer->data, message, MESSAGE_DATA_SIZE);

    msgsnd(serverQueueID, messageBuffer, sizeof(MessageBuffer), 0);
}

void action_2ALL(char *message) {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));

    messageBuffer->type = SERVER_TO_ALL;
    messageBuffer->clientID = clientID;
    strncpy(messageBuffer->data, message, MESSAGE_DATA_SIZE);

    msgsnd(serverQueueID, messageBuffer, sizeof(MessageBuffer), 0);
}

void action_STOP() {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));

    messageBuffer->type = COMMON_STOP;
    messageBuffer->clientID = clientID;

    msgsnd(serverQueueID, messageBuffer, sizeof(MessageBuffer), 0);
    msgctl(clientQueueID, IPC_RMID, NULL);
    free(messageBuffer);

    printf("EXITING...\n");
    exit(EXIT_SUCCESS);
}

void handle_MESSAGE() {
    MessageBuffer *messageBuffer = malloc(sizeof(MessageBuffer));
    while (msgrcv(clientQueueID, messageBuffer, sizeof(MessageBuffer), 0, IPC_NOWAIT) >= 0) {
        if (messageBuffer->type == COMMON_STOP) {
            printf("STOP: server is closing\n");
            action_STOP();
        } else if (messageBuffer->type == CLIENT_MESSAGE) {
            printf("MESSAGE: from %d \"%s\"\n", messageBuffer->clientID, messageBuffer->data);
        } else {
            fprintf(stderr, "Received invalid message\n");
        }
    }
}
