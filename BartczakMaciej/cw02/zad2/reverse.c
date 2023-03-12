#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1024
#endif

char *reverseString(char *str) {
    unsigned long len = strlen(str);
    char *copy = calloc(len + 1, sizeof(char));
    strcpy(copy, str);

    for (int i = 0; i < len; ++i) {
        char c = copy[len - i - 1];
        str[i] = c;
    }

    free(copy);
    return str;
}

int reverse(char *sourcePath, char *destinationPath) {
    char buffer[BLOCK_SIZE + 1];

    FILE *src = fopen(sourcePath, "r");
    FILE *dest = fopen(destinationPath, "w");

    fseek(src, 0, SEEK_END);
    long srcSize = ftell(src);
    fseek(src, 0, SEEK_SET);

    size_t fullBlocks = (size_t) (srcSize / BLOCK_SIZE);
    size_t remainder = (size_t) (srcSize % BLOCK_SIZE);

    size_t blocksRead;

    for (int i = 1; i <= fullBlocks; ++i) {
        fseek(src, -1 * (BLOCK_SIZE * i), SEEK_END);
        blocksRead = fread(buffer, sizeof(char), BLOCK_SIZE, src);
        buffer[blocksRead] = '\0';
        fwrite(reverseString(buffer), sizeof(char), blocksRead, dest);
    }

    fseek(src, 0, SEEK_SET);
    blocksRead = fread(buffer, sizeof(char), remainder, src);
    buffer[blocksRead] = '\0';
    fwrite(reverseString(buffer), sizeof(char), blocksRead, dest);
    fwrite("\n", sizeof(char), 1, dest);

    fclose(src);
    fclose(dest);

    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    char *sourcePath = argv[1];
    char *destinationPath = argv[2];

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    int success = reverse(sourcePath, destinationPath);
    if (!success) {
        fprintf(stderr, "Error number: %d\n", errno);
        return EXIT_FAILURE;
    }

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    printf("BLOCK_SIZE=%d, elapsed real time: %ld seconds, %ld microseconds\n",
           BLOCK_SIZE,
           endTime.tv_sec - startTime.tv_sec,
           endTime.tv_usec - startTime.tv_usec);

    return EXIT_SUCCESS;
}