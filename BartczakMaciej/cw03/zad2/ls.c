#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "One argument should be supplied\n");
        return EXIT_FAILURE;
    }

    char *name = argv[0];
    char *path = argv[1];
    printf("%s ", name);
    fflush(stdout);

    execl("/bin/ls", "ls", path, NULL);
    return EXIT_SUCCESS;
}