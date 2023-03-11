#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>

int replace_lib(const char *sourcePath, const char *destinationPath, const char toReplace, const char replaceWith) {
    char *buffer = NULL;
    FILE *file = NULL;

    file = fopen(sourcePath, "r");
    if (file == NULL) {
        fprintf(stderr, "An error occurred while opening input file\n");
        goto error;
    }

    if (fseek(file, 0, SEEK_END)) {
        fprintf(stderr, "An error occurred while processing input file\n");
        goto error;
    }
    long srcSize = ftell(file);
    if (fseek(file, 0, SEEK_SET)) {
        fprintf(stderr, "An error occurred while processing input file\n");
        goto error;
    }

    buffer = calloc(srcSize, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "An error occurred during dynamic memory allocation\n");
        goto error;
    }

    fread(buffer, sizeof(char), srcSize, file);
    if (ferror(file)) {
        fprintf(stderr, "An error occurred while reading input file\n");
        goto error;
    }

    if (fclose(file)) {
        fprintf(stderr, "An error occurred while closing input file\n");
        goto error;
    }

    for (int i = 0; i < srcSize; ++i) {
        if (buffer[i] == toReplace) {
            buffer[i] = replaceWith;
        }
    }

    file = fopen(destinationPath, "w");
    if (file == NULL) {
        fprintf(stderr, "An error occurred while opening output file\n");
        goto error;
    }

    fwrite(buffer, sizeof(char), srcSize, file);
    if (ferror(file)) {
        fprintf(stderr, "An error occurred while writing to output file\n");
        goto error;
    }

    if (fclose(file)) {
        fprintf(stderr, "An error occurred while closing output file\n");
        goto error;
    }

    free(buffer);
    return true;

    error:
    if (buffer != NULL) {
        free(buffer);
    }
    if (file != NULL) {
        fclose(file);
    }
    return false;
}

int replace_sys(const char *sourcePath, const char *destinationPath, const char toReplace, const char replaceWith) {
    char *buffer = NULL;
    int file = -1;

    file = open(sourcePath, O_RDONLY);
    if (file == -1) {
        fprintf(stderr, "An error occurred while opening input file\n");
        goto error;
    }

    struct stat st;
    if (fstat(file, &st) == -1) {
        fprintf(stderr, "An error occurred while processing input file\n");
        goto error;
    }

    unsigned long srcSize = st.st_size;
    buffer = calloc(srcSize, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "An error occurred during dynamic memory allocation\n");
        goto error;
    }

    if (read(file, buffer, sizeof(char) * srcSize) == -1) {
        fprintf(stderr, "An error occurred while reading input file\n");
        goto error;
    }
    if (close(file) == -1) {
        fprintf(stderr, "An error occurred while closing input file\n");
        goto error;
    }

    for (int i = 0; i < srcSize; ++i) {
        if (buffer[i] == toReplace) {
            buffer[i] = replaceWith;
        }
    }

    file = open(destinationPath, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == -1) {
        fprintf(stderr, "An error occurred while opening output file\n");
        goto error;
    }

    if (write(file, buffer, sizeof(char) * srcSize) == -1) {
        fprintf(stderr, "An error occurred while writing to output file\n");
        goto error;
    }
    if (close(file) == -1) {
        fprintf(stderr, "An error occurred while closing output file\n");
        goto error;
    }

    return true;

    error:
    if (buffer != NULL) {
        free(buffer);
    }
    if (file != -1) {
        close(file);
    }
    return false;
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


#ifdef SYS
    char *version = "SYS";
    int success = replace_sys(sourcePath, destinationPath, toReplace, replaceWith);
#else
    char *version = "LIB";
    int success = replace_lib(sourcePath, destinationPath, toReplace, replaceWith);
#endif

    if (!success) {
        fprintf(stderr, "Error number: %d\n", errno);
        return 1;
    }

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    printf("%s version, elapsed real time: %ld seconds, %ld microseconds\n",
           version,
           endTime.tv_sec - startTime.tv_sec,
           endTime.tv_usec - startTime.tv_usec);
    return 0;
}
