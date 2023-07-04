#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        perror("connect");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Enter filename: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == 0) {
            break;
        }

        send(sock, buffer, strlen(buffer), 0);

        long fileSize;
        read(sock, &fileSize, sizeof(fileSize));
        if (fileSize <= 0) {
            printf("That file doesn't exist\n");
            continue;
        }

        char *filename = buffer;
        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
            perror("fopen");
            return EXIT_FAILURE;
        }

        ssize_t accumulator = 0;
        while (accumulator < fileSize) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytesRead = read(sock, buffer, BUFFER_SIZE);
            accumulator += bytesRead;
            if (bytesRead > 0) {
                fwrite(buffer, 1, bytesRead, fp);
            }
            if (bytesRead < BUFFER_SIZE) {
                if (ferror(fp)) {
                    printf("Error in reading file\n");
                }
                break;
            }
        }

        if (accumulator == fileSize) {
            printf("File download completed\n");
        }

        fclose(fp);
    }

    close(sock);
    return EXIT_SUCCESS;
}
