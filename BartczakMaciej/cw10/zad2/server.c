#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "message.h"

#define MAX_CONN 16
#define PING_INTERVAL 20

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int epollFD;

typedef union {
    struct sockaddr_un uni;
    struct sockaddr_in web;
} AddrUnion;

typedef enum {
    empty = 0, init, ready
} ClientStateEnum;

typedef struct {
    AddrUnion addr;
    size_t addrLen;
    int sock;
    ClientStateEnum state;
    struct client *peer;
    char nickname[NICKNAME_LEN];
    bool responding;
} ClientStruct;

typedef enum {
    socket_event, client_event
} EventTypeEnum;

typedef union {
    ClientStruct *client;
    int socket;
} PayloadUnion;

typedef struct {
    EventTypeEnum type;
    PayloadUnion payload;
} EventDataStruct;

ClientStruct clients[MAX_CONN];


void deleteClient(ClientStruct *client) {
    printf("Deleting %s\n", client->nickname);
    message msg = {.type = DISCONNECT};

    sendto(client->sock, &msg, sizeof msg, 0, (struct sockaddr *) &client->addr, client->addrLen);
    memset(&client->addr, 0, sizeof client->addr);

    client->state = empty;
    client->sock = 0;
    client->nickname[0] = '\0';
}


void sendMessage(ClientStruct *client, MessageType type, char text[MESSAGE_LEN]) {
    message msg;
    msg.type = type;
    memcpy(&msg.text, text, MESSAGE_LEN * sizeof(char));

    sendto(client->sock, &msg, sizeof msg, 0, (struct sockaddr *) &client->addr, client->addrLen);
}


void onClientMessage(ClientStruct *client, message *msgPtr) {
    message mess = *msgPtr;

    printf("Got message %i %s\n", (int) mess.type, mess.text);

    if (mess.type == PING) {
        pthread_mutex_lock(&mutex);

        printf("pong %s\n", client->nickname);
        client->responding = true;

        pthread_mutex_unlock(&mutex);
    } else if (mess.type == DISCONNECT || mess.type == STOP) {
        pthread_mutex_lock(&mutex);

        deleteClient(client);

        pthread_mutex_unlock(&mutex);
    } else if (mess.type == TO_ALL) {
        char out[256] = "";
        strcat(out, client->nickname);
        strcat(out, ": ");
        strcat(out, mess.text);

        for (int i = 0; i < MAX_CONN; i++) {
            if (clients[i].state != empty) {
                sendMessage(clients + i, GET, out);
            }
        }
    } else if (mess.type == LIST) {
        for (int i = 0; i < MAX_CONN; ++i) {
            if (clients[i].state != empty) {
                sendMessage(client, GET, clients[i].nickname);
            }
        }
    } else if (mess.type == TO_ONE) {
        char out[256] = "";
        strcat(out, client->nickname);
        strcat(out, ": ");
        strcat(out, mess.text);

        for (int i = 0; i < MAX_CONN; ++i) {
            if (clients[i].state != empty) {
                if (strcmp(clients[i].nickname, mess.nickname) == 0) {
                    sendMessage(clients + i, GET, out);
                }
            }
        }
    }
}

void initSocket(int socket, void *addr, int addr_size) {
    if (bind(socket, (struct sockaddr *) addr, addr_size) == -1) {
        fprintf(stderr, "bind, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct epoll_event event = {
            .events = EPOLLIN | EPOLLPRI,
            .data = {.fd = socket}
    };

    epoll_ctl(epollFD, EPOLL_CTL_ADD, socket, &event);
}


void newClient(AddrUnion *addr, socklen_t addrlen, int sock, char *nickname) {
    pthread_mutex_lock(&mutex);
    int emptyIndex = -1;

    for (int i = 0; i < MAX_CONN; ++i) {
        if (clients[i].state == empty) {
            emptyIndex = i;
        } else if (strncmp(nickname, clients[i].nickname, sizeof clients->nickname) == 0) {
            pthread_mutex_unlock(&mutex);
            message msg = {.type = USERNAME_TAKEN};
            printf("Nickname %s already taken\n", nickname);
            sendto(sock, &msg, sizeof msg, 0, (struct sockaddr *) addr, addrlen);
            return;
        }
    }

    if (emptyIndex == -1) {
        pthread_mutex_unlock(&mutex);
        printf("Server is at capacity\n");
        message msg = {.type = SERVER_FULL};
        sendto(sock, &msg, sizeof msg, 0, (struct sockaddr *) addr, addrlen);
        return;
    }

    printf("New ClientStruct %s\n", nickname);

    ClientStruct *client = &clients[emptyIndex];
    memcpy(&client->addr, addr, addrlen);
    client->addrLen = addrlen;
    client->state = init;
    client->responding = true;
    client->sock = sock;

    memset(client->nickname, 0, sizeof client->nickname);
    strncpy(client->nickname, nickname, sizeof client->nickname - 1);

    pthread_mutex_unlock(&mutex);
}


void *ping() {
    message msg = {.type = PING};

    while (1) {
        sleep(PING_INTERVAL);
        pthread_mutex_lock(&mutex);
        printf("Pinging clients\n");

        for (int i = 0; i < MAX_CONN; i++) {
            if (clients[i].state != empty) {
                if (clients[i].responding) {
                    clients[i].responding = false;
                    sendto(clients[i].sock, &msg, sizeof msg, 0, (struct sockaddr *) &clients[i].addr,
                           clients[i].addrLen);
                } else
                    deleteClient(&clients[i]);
            }
        }

        pthread_mutex_unlock(&mutex);
    }
}


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Wrong arguments. Usage: [port] [path]\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char *socketPath = argv[2];

    epollFD = epoll_create1(0);
    if (epollFD == -1) {
        fprintf(stderr, "epoll_create1, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un localAddress = {.sun_family = AF_UNIX};
    strncpy(localAddress.sun_path, socketPath, sizeof localAddress.sun_path);

    struct sockaddr_in webAddress = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = {.s_addr = htonl(INADDR_ANY)},
    };

    unlink(socketPath);
    int localSock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (localSock == -1) {
        fprintf(stderr, "socket, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    initSocket(localSock, &localAddress, sizeof localAddress);

    int webSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (webSock == -1) {
        fprintf(stderr, "socket, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    initSocket(webSock, &webAddress, sizeof webAddress);

    pthread_t pingThread;
    pthread_create(&pingThread, NULL, ping, NULL);

    printf("Server is listening on: %d, '%s'\n", port, socketPath);

    struct epoll_event events[10];

    while (1) {
        int n = epoll_wait(epollFD, events, 10, -1);
        if (n == -1) {
            fprintf(stderr, "epoll_wait, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n; i++) {
            int sock = events[i].data.fd;
            message msg;
            AddrUnion addr;
            socklen_t addrLen = sizeof addr;
            recvfrom(sock, &msg, sizeof msg, 0, (struct sockaddr *) &addr, &addrLen);

            if (msg.type == CONNECT) {
                newClient(&addr, addrLen, sock, msg.nickname);
            } else {
                for (int j = 0; j < MAX_CONN; ++j) {
                    if (memcmp(&clients[j].addr, &addr, addrLen) == 0) {
                        onClientMessage(&clients[j], &msg);
                        break;
                    }
                }
            }
        }
    }
}