#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

int printFiles(char *directoryPath) {
    DIR *dir = NULL;

    dir = opendir(directoryPath);
    if (dir == NULL) {
        fprintf(stderr, "An error occurred while opening root directory\n");
        goto error;
    }

    long long totalBytes = 0;
    struct dirent *dirent;
    struct stat statBuffer;
    while ((dirent = readdir(dir)) != NULL) {
        if (stat(dirent->d_name, &statBuffer) == -1) {
            fprintf(stderr, "An error occurred while getting file attributes\n");
            goto error;
        }
        if (S_ISDIR(statBuffer.st_mode)) {
            continue;
        }

        printf("%ld\t%s\n", statBuffer.st_size, dirent->d_name);
        totalBytes += statBuffer.st_size;
    }
    if (dirent == (struct dirent *) -1) {
        fprintf(stderr, "An error occurred while reading root directory\n");
        goto error;
    }

    printf("%lld\t%s\n", totalBytes, "total");
    closedir(dir);
    return EXIT_SUCCESS;

    error:
    if (dir != NULL) {
        closedir(dir);
    }
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    char *directoryPath = argv[1];

    int failure = printFiles(directoryPath);
    if (failure) {
        fprintf(stderr, "Error number: %d\n", errno);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
