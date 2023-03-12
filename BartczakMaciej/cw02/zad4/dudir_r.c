#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ftw.h>

long long totalBytes = 0;

int fn(const char *path, const struct stat *sb, int typeFlag) {
    if (sb == NULL || path == NULL) {
        return EXIT_FAILURE;
    }
    if (S_ISDIR(sb->st_mode)) {
        return EXIT_SUCCESS;
    }

    printf("%ld\t%s\n", sb->st_size, path);
    totalBytes += sb->st_size;

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    char *directoryPath = argv[1];

    totalBytes = 0;
    int failure = ftw(directoryPath, fn, 1);
    if (failure) {
        return EXIT_FAILURE;
    }

    printf("%lld\t%s\n", totalBytes, "total");
    return EXIT_SUCCESS;
}
