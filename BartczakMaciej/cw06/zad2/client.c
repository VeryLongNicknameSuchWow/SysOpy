#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <mqueue.h>
#include <fcntl.h>
#include "common.h"

#define INPUT_BUFFER_SIZE 1024

mqd_t clientQueueDescriptor;
mqd_t serverQueueDescriptor;
int clientID;
char clientQueueName[32];

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
    snprintf(clientQueueName, sizeof(clientQueueName), "/client_queue_%d", rand() % 10000 + 1000);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(MessageBuffer);
    attr.mq_curmsgs = 0;

    clientQueueDescriptor = mq_open(clientQueueName, O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);
    serverQueueDescriptor = mq_open(SERVER_QUEUE_NAME, O_WRONLY | O_NONBLOCK);

    action_INIT();

    if (clientID < 0) {
        fprintf(stderr, "Received invalid clientID, server may be atcapacity\n");
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
    MessageBuffer messageBuffer;

    messageBuffer.type = COMMON_INIT;
    strncpy(messageBuffer.data, clientQueueName, MESSAGE_DATA_SIZE);

    mq_send(serverQueueDescriptor, (const char *) &messageBuffer, sizeof(MessageBuffer), 0);
    mq_receive(clientQueueDescriptor, (char *) &messageBuffer, sizeof(MessageBuffer), NULL);

    int newClientID = messageBuffer.clientID;

    printf("INIT: received clientID=%d\n", newClientID);
    clientID = newClientID;
}

void action_LIST() {
    MessageBuffer messageBuffer;

    messageBuffer.type = SERVER_LIST;
    messageBuffer.clientID = clientID;

    mq_send(serverQueueDescriptor, (const char *) &messageBuffer, sizeof(MessageBuffer), 0);
    mq_receive(clientQueueDescriptor, (char *) &messageBuffer, sizeof(MessageBuffer), NULL);

    printf("LIST: %s\n", messageBuffer.data);
}

void action_2ONE(int destination, char *message) {
    MessageBuffer messageBuffer;

    messageBuffer.type = SERVER_TO_ONE;
    messageBuffer.clientID = clientID;
    messageBuffer.destinationID = destination;
    strncpy(messageBuffer.data, message, MESSAGE_DATA_SIZE);

    mq_send(serverQueueDescriptor, (const char *) &messageBuffer, sizeof(MessageBuffer), 0);
}

void action_2ALL(char *message) {
    MessageBuffer messageBuffer;

    messageBuffer.type = SERVER_TO_ALL;
    messageBuffer.clientID = clientID;
    strncpy(messageBuffer.data, message, MESSAGE_DATA_SIZE);

    mq_send(serverQueueDescriptor, (const char *) &messageBuffer, sizeof(MessageBuffer), 0);
}

void action_STOP() {
    MessageBuffer messageBuffer;

    messageBuffer.type = COMMON_STOP;
    messageBuffer.clientID = clientID;

    mq_send(serverQueueDescriptor, (const char *) &messageBuffer, sizeof(MessageBuffer), 0);
    mq_close(clientQueueDescriptor);
    mq_unlink(clientQueueName);

    printf("EXITING...\n");
    exit(EXIT_SUCCESS);
}

void handle_MESSAGE() {
    MessageBuffer messageBuffer;
    while (mq_receive(clientQueueDescriptor, (char *) &messageBuffer, sizeof(MessageBuffer), NULL) >= 0) {
        if (messageBuffer.type == COMMON_STOP) {
            printf("STOP: server stopped\n");
            action_STOP();
            exit(EXIT_SUCCESS);
        } else {
            printf("Message from client %d: %s\n", messageBuffer.clientID, messageBuffer.data);
        }
    }
}
