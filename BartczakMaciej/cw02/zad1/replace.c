//
// Created by rynbou on 3/11/23.
//

#include "replace.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

int replace(const char *sourcePath, const char *destinationPath, const char toReplace, const char replaceWith) {
#ifndef SYS
    FILE *file = fopen(sourcePath, "r");

    fseek(file, 0, SEEK_END);
    long srcSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char buffer[srcSize];
    fread(buffer, sizeof(char), srcSize, file);
    fclose(file);

    for (int i = 0; i < srcSize; ++i) {
        if (buffer[i] == toReplace) {
            buffer[i] = replaceWith;
        }
    }

    file = fopen(destinationPath, "w");
    fwrite(buffer, sizeof(char), srcSize, file);
    fclose(file);
#else
    int file = open(sourcePath, O_RDONLY);

    struct stat st;
    fstat(file, &st);
    unsigned long srcSize = st.st_size;

    char buffer[srcSize];
    read(file, buffer, sizeof(char) * srcSize);
    close(file);

    for (int i = 0; i < srcSize; ++i) {
        if (buffer[i] == toReplace) {
            buffer[i] = replaceWith;
        }
    }

    file = open(destinationPath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    write(file, buffer, sizeof(char) * srcSize);
    close(file);
#endif

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        return 1;
    }

    char toReplace = argv[1][0];
    char replaceWith = argv[2][0];
    char *sourcePath = argv[3];
    char *destinationPath = argv[4];

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    replace(sourcePath, destinationPath, toReplace, replaceWith);

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

#ifdef SYS
    char *version = "SYS";
#else
    char *version = "LIB";
#endif

    printf("%s version, elapsed real time: %ld seconds, %ld microseconds\n",
           version,
           endTime.tv_sec - startTime.tv_sec,
           endTime.tv_usec - startTime.tv_usec);
    return 0;
}
