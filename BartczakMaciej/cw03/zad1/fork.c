#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "One argument should be supplied\n");
        return EXIT_FAILURE;
    }

    long n = strtol(argv[1], NULL, 10);
    if (n <= 0) {
        fprintf(stderr, "Invalid number supplied\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) {
        if (fork() == 0) {
            printf("PID: %d\t PPID: %d\n", getpid(), getppid());
            return EXIT_SUCCESS;
        }
    }

    while (wait(NULL) > 0);
    printf("%ld\n", n);
    return EXIT_SUCCESS;
}