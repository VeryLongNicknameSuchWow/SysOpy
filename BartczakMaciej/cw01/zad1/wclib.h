//
// Created by rynbou on 3/3/23.
//

#ifndef SYSOPY_WCLIB_H
#define SYSOPY_WCLIB_H

#define BUFFER_SIZE 1024

#include <stdlib.h>

typedef struct {
    char *wcResult;
} FileInfo;

typedef struct {
    FileInfo **fileInfos;
    size_t currentSize;
    size_t maxSize;
} FileInfoContainer;

FileInfoContainer *createContainer(size_t maxSize);

void removeContainer(FileInfoContainer *container);

void saveFileInfo(FileInfoContainer *container, char *fileName);

FileInfo *getFileInfo(FileInfoContainer *container, int index);

void removeFileInfo(FileInfoContainer *container, int index);

#endif //SYSOPY_WCLIB_H
