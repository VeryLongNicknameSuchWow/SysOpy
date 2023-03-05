//
// Created by rynbou on 3/3/23.
//
#include <string.h>
#include <stdio.h>

#include "wclib.h"


FileInfoContainer *createContainer(size_t maxSize) {
    FileInfoContainer *result = malloc(sizeof(FileInfoContainer));
    if (result == NULL) {
        return NULL;
    }

    result->fileInfos = calloc(maxSize, sizeof(FileInfo *));
    if (result->fileInfos == NULL) {
        return NULL;
    }

    result->maxSize = maxSize;
    result->currentSize = 0;

    return result;
}

void removeContainer(FileInfoContainer *container) {
    free(container->fileInfos);
    free(container);
}

void saveFileInfo(FileInfoContainer *container, char *fileName) {
    char *tempPath = tempnam("/tmp", "wc_res");
    if (tempPath == NULL) {
        return;
    }

    char command[BUFFER_SIZE];
    strcpy(command, "wc ");
    strcat(command, fileName);
    strcat(command, " 1>> ");
    strcat(command, tempPath);
    system(command);

    FILE *tempFile = fopen(tempPath, "r");
    if (tempFile == NULL) {
        return;
    }

    char buffer[BUFFER_SIZE];
    if (fgets(buffer, BUFFER_SIZE, tempFile) == NULL) {
        return;
    }
    size_t size = strlen(buffer);

    FileInfo *fileInfo = malloc(sizeof(FileInfo));
    if (fileInfo == NULL) {
        return;
    }
    fileInfo->wcResult = calloc(sizeof(char), size);
    if (fileInfo->wcResult == NULL) {
        return;
    }

    strcpy(fileInfo->wcResult, buffer);
    container->fileInfos[container->currentSize++] = fileInfo;
}

FileInfo *getFileInfo(FileInfoContainer *container, int index) {
    return container->fileInfos[index];
}

void removeFileInfo(FileInfoContainer *container, int index) {
    free(container->fileInfos[index]->wcResult);
}
