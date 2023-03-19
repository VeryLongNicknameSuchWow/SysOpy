#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_PATTERN_SIZE 255
size_t patternSize = 0;
char *pattern = "\0";

int checkFile(char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s (%s)\n", filePath, strerror(errno));
        fflush(stderr);
        return EXIT_FAILURE;
    }

    char buffer[patternSize + 1];
    size_t blocksRead = fread(buffer, sizeof(char), patternSize, file);
    buffer[blocksRead] = '\0';
    if (ferror(file)) {
        fprintf(stderr, "Could not read file: %s (%s)\n", filePath, strerror(errno));
        fflush(stderr);
        return EXIT_FAILURE;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close file: %s (%s)\n", filePath, strerror(errno));
        fflush(stderr);
        return EXIT_FAILURE;
    }

    if (strcmp(buffer, pattern) == 0) {
        printf("PID: %d\t%s\n", getpid(), filePath);
        fflush(stdout);
    }
    return EXIT_SUCCESS;
}

int traverseDir(char *dirPath) {
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        fprintf(stderr, "Could not open directory: %s (%s)\n", dirPath, strerror(errno));
        fflush(stderr);
        return EXIT_FAILURE;
    }

    int fileExitCode = EXIT_SUCCESS;
    struct dirent *dirent;
    struct stat statBuffer;
    while ((dirent = readdir(dir)) != NULL) {
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            continue;
        }
        size_t pathSize = strlen(dirPath) + strlen(dirent->d_name) + 2; //including additional / and \0
        if (pathSize > PATH_MAX) {
            fprintf(stderr, "Path is too long, skipping branch\n");
            fflush(stderr);
            continue;
        }

        char pathBuffer[pathSize];
        sprintf(pathBuffer, "%s/%s", dirPath, dirent->d_name);

        if (stat(pathBuffer, &statBuffer) == -1) {
            fprintf(stderr, "An error occurred while getting stats of: %s (%s)\n", dirent->d_name, strerror(errno));
            fflush(stderr);
            return EXIT_FAILURE;
        }

        if (S_ISDIR(statBuffer.st_mode)) {
            if (fork() == 0) {
                return traverseDir(pathBuffer);
            }
        } else {
            fileExitCode |= checkFile(pathBuffer);
        }
    }
    if (dirent == (struct dirent *) -1) {
        fprintf(stderr, "An error occurred while reading directory: %s (%s)\n", dirPath, strerror(errno));
        fflush(stderr);
        return EXIT_FAILURE;
    }

    return fileExitCode;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Invalid argument count\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    char *dirPath = argv[1];
    struct stat st;
    if (stat(dirPath, &st) == -1) {
        fprintf(stderr, "Could not open the supplied directory (%s)\n", strerror(errno));
        fflush(stderr);
        return EXIT_FAILURE;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Supplied path is not a directory\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    pattern = argv[2];
    patternSize = strlen(pattern);
    if (patternSize > MAX_PATTERN_SIZE) {
        fprintf(stderr, "Pattern argument is too long\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    int exitCode = traverseDir(dirPath);
    while (wait(NULL) > 0);

    return exitCode;
}
