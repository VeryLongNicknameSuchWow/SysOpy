#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

double f(double x) {
    return 4 / (x * x + 1);
}

int main(int argc, char *argv[]) {
    struct timeval tStart, tEnd, tElapsed;
    gettimeofday(&tStart, NULL);

    if (argc != 3) {
        fprintf(stderr, "Invalid argument count!\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }
    char *dxStr = argv[1];
    double dx = strtod(dxStr, NULL);
    if (dx <= 0 || errno == ERANGE) {
        fprintf(stderr, "Invalid dx argument!\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }
    long n = strtol(argv[2], NULL, 10);
    if (dx <= 0 || errno == ERANGE || errno == EINVAL) {
        fprintf(stderr, "Invalid process count argument!\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    double a = 0;
    double b = 1;
    double px = (b - a) / (double) n;

    char fifoPath[BUFFER_SIZE];
    snprintf(fifoPath, BUFFER_SIZE, "/tmp/fifo_%d", getpid());
    if (mkfifo(fifoPath, 0666) == -1) {
        fprintf(stderr, "Could not create FIFO! (%s)\n", strerror(errno));
        goto failure;
    }

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            char start[BUFFER_SIZE], end[BUFFER_SIZE], idx[BUFFER_SIZE];
            snprintf(start, BUFFER_SIZE, "%f", a + i * px);
            snprintf(end, BUFFER_SIZE, "%f", a + (i + 1) * px);
            snprintf(idx, BUFFER_SIZE, "%d", i);
            execl("./child", "./child", fifoPath, start, end, dxStr, NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
    }

    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Could not open FIFO! (%s)\n", strerror(errno));
        goto failure;
    }

    while (wait(NULL) > 0);

    double result = 0;
    for (int i = 0; i < n; ++i) {
        double res;
        if (read(fd, &res, sizeof(double)) == -1) {
            fprintf(stderr, "Could not read pipe! (%s)\n", strerror(errno));
            goto failure;
        }
        result += res;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Could not close pipe (%s)\n", strerror(errno));
        goto failure;
    }
    if (unlink(fifoPath) == -1) {
        fprintf(stderr, "Could not remove FIFO (%s)\n", strerror(errno));
        goto failure;
    }

    gettimeofday(&tEnd, NULL);
    timersub(&tEnd, &tStart, &tElapsed);
    printf("result=%f;sec=%ld,usec=%ld\n", result, tElapsed.tv_sec, tElapsed.tv_usec);

    fflush(NULL);
    return EXIT_SUCCESS;

    failure:
    fflush(NULL);
    return EXIT_FAILURE;
}
