#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main() {
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD == 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in address;
    socklen_t addrLen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFD, (struct sockaddr *) &address, addrLen) < 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    if (listen(serverFD, 3) < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    int clientFD = accept(serverFD, (struct sockaddr *) &address, &addrLen);
    if (clientFD < 0) {
        perror("accept");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        read(clientFD, buffer, BUFFER_SIZE - 1);
        char *filename = buffer;
        if (strlen(filename) == 0) {
            break;
        }

        long size = -1;
        FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            printf("Could not open '%s' file\n", filename);
            write(clientFD, &size, sizeof(size));
            continue;
        }

        if (fseek(fp, 0, SEEK_END) < 0) {
            perror("fseek");
            return EXIT_FAILURE;
        }
        size = ftell(fp);
        if (fseek(fp, 0, SEEK_SET) < 0) {
            perror("fseek");
            return EXIT_FAILURE;
        }

        write(clientFD, &size, sizeof(size));

        while (1) {
            size_t bytesRead = fread(buffer, 1, BUFFER_SIZE, fp);
            if (bytesRead > 0) {
                write(clientFD, buffer, bytesRead);
            }
            if (bytesRead < BUFFER_SIZE) {
                if (feof(fp)) {
                    printf("File transfer completed\n");
                }
                if (ferror(fp)) {
                    printf("Error in reading file\n");
                }
                break;
            }
        }

        fclose(fp);
    }
    close(clientFD);
    close(serverFD);
    return EXIT_SUCCESS;
}
