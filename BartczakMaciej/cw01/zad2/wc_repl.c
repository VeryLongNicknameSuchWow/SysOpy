//
// Created by rynbou on 3/3/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../zad1/wclib.h"

int main() {
    FileInfoContainer *container = NULL;
    char buffer[BUFFER_SIZE];
    char *substring;
    const char *delim = " \n";

    struct timeval startTime;
    struct timeval endTime;
    struct rusage startRusage;
    struct rusage endRusage;

    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        substring = strtok(buffer, delim);
        gettimeofday(&startTime, NULL);
        getrusage(RUSAGE_SELF, &startRusage);

        if (strcmp(substring, "init") == 0) {
            substring = strtok(NULL, delim);
            int size = atoi(substring);
            container = createContainer(size);
        } else if (strcmp(substring, "count") == 0) {
            substring = strtok(NULL, delim);
            saveFileInfo(container, substring);
        } else if (strcmp(substring, "show") == 0) {
            substring = strtok(NULL, delim);
            int index = atoi(substring);
            FileInfo *fileInfo = getFileInfo(container, index);
            printf("Contents of container at index %d:\n%s\n", index, fileInfo->wcResult);
        } else if (strcmp(substring, "delete") == 0) {
            substring = strtok(NULL, delim);
            substring = strtok(NULL, delim);
            int index = atoi(substring);
            removeFileInfo(container, index);
        } else if (strcmp(substring, "destroy") == 0) {
            removeContainer(container);
        }

        gettimeofday(&endTime, NULL);
        getrusage(RUSAGE_SELF, &endRusage);

        long seconds = endTime.tv_sec - startTime.tv_sec;
        long microseconds = endTime.tv_usec - startTime.tv_usec;
        printf("Elapsed real time: %ld seconds, %ld microseconds\n", seconds, microseconds);

        long userSeconds = endRusage.ru_utime.tv_sec - startRusage.ru_utime.tv_sec;
        long userMicroseconds = endRusage.ru_utime.tv_usec - startRusage.ru_utime.tv_usec;
        printf("User CPU time: %ld seconds, %ld microseconds\n", userSeconds, userMicroseconds);

        long systemSeconds = endRusage.ru_stime.tv_sec - startRusage.ru_stime.tv_sec;
        long systemMicroseconds = endRusage.ru_stime.tv_usec - startRusage.ru_stime.tv_usec;
        printf("System CPU time: %ld seconds, %ld microseconds\n", systemSeconds, systemMicroseconds);
        printf("\n");
    }
}