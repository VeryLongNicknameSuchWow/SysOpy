#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1024
#endif

char *reverseString(char *str) {
    size_t len = strlen(str);
    char copy[len + 1];
    strcpy(copy, str);

    for (size_t i = 0; i < len; ++i) {
        str[i] = copy[len - i - 1];
    }

    return str;
}

int reverse(char *sourcePath, char *destinationPath) {
    char buffer[BLOCK_SIZE + 1];
    FILE *src = NULL;
    FILE *dest = NULL;

    src = fopen(sourcePath, "r");
    if (src == NULL) {
        fprintf(stderr, "An error occurred while opening source file\n");
        goto error;
    }

    dest = fopen(destinationPath, "w");
    if (dest == NULL) {
        fprintf(stderr, "An error occurred while opening destination file\n");
        goto error;
    }

    if (fseek(src, 0, SEEK_END)) {
        fprintf(stderr, "An error occurred while processing source file\n");
        goto error;
    }
    long srcSize = ftell(src);
    if (fseek(src, 0, SEEK_SET)) {
        fprintf(stderr, "An error occurred while processing source file\n");
        goto error;
    }

    size_t fullBlocks = (size_t) (srcSize / BLOCK_SIZE);
    size_t lastBlock = (size_t) (srcSize % BLOCK_SIZE);

    size_t blocksRead;

    for (int i = 1; i <= fullBlocks; ++i) {
        if (fseek(src, -1 * (BLOCK_SIZE * i), SEEK_END)) {
            fprintf(stderr, "An error occurred while processing source file\n");
            goto error;
        }
        blocksRead = fread(buffer, sizeof(char), BLOCK_SIZE, src);
        if (ferror(src)) {
            fprintf(stderr, "An error occurred while reading source file\n");
            goto error;
        }
        buffer[blocksRead] = '\0';
        fwrite(reverseString(buffer), sizeof(char), blocksRead, dest);
        if (ferror(dest)) {
            fprintf(stderr, "An error occurred while writing to destination file\n");
            goto error;
        }
    }

    if (fseek(src, 0, SEEK_SET)) {
        fprintf(stderr, "An error occurred while processing source file\n");
        goto error;
    }
    blocksRead = fread(buffer, sizeof(char), lastBlock, src);
    if (ferror(src)) {
        fprintf(stderr, "An error occurred while reading source file\n");
        goto error;
    }
    buffer[blocksRead] = '\0';
    fwrite(reverseString(buffer), sizeof(char), blocksRead, dest);
    if (ferror(dest)) {
        fprintf(stderr, "An error occurred while writing to destination file\n");
        goto error;
    }
    fwrite("\n", sizeof(char), 1, dest);
    if (ferror(dest)) {
        fprintf(stderr, "An error occurred while writing to destination file\n");
        goto error;
    }

    if (fclose(src)) {
        fprintf(stderr, "An error occurred while closing source file\n");
        goto error;
    }
    if (fclose(dest)) {
        fprintf(stderr, "An error occurred while closing source file\n");
        goto error;
    }
    return EXIT_SUCCESS;

    error:
    if (src != NULL) {
        fclose(src);
    }
    if (dest != NULL) {
        fclose(dest);
    }
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    char *sourcePath = argv[1];
    char *destinationPath = argv[2];

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    int failure = reverse(sourcePath, destinationPath);
    if (failure) {
        fprintf(stderr, "Error number: %d\n", errno);
        return EXIT_FAILURE;
    }

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    struct timeval elapsedTime;
    timersub(&endTime, &startTime, &elapsedTime);

    printf("BLOCK_SIZE=%d, elapsed real time: %ld seconds, %ld microseconds\n",
           BLOCK_SIZE,
           elapsedTime.tv_sec,
           elapsedTime.tv_usec);

    return EXIT_SUCCESS;
}