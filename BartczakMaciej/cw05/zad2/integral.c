#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024

double f(double x) {
    return 4 / (x * x + 1);
}

double integrate(double (*f)(double), double a, double b, double dx) {
    double result = 0;
    for (double x = a; x < b; x += dx) {
        result += f(x) * dx;
    }
    return result;
}

int main(int argc, char *argv[]) {
    struct timeval tStart, tEnd, tElapsed;
    gettimeofday(&tStart, NULL);

    if (argc != 3) {
        fprintf(stderr, "Invalid argument count!\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }
    double dx = strtod(argv[1], NULL);
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

    int *pipes = calloc(n, sizeof(int));
    if (pipes == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        goto failure;
    }

    for (int i = 0; i < n; ++i) {
        int fd[2];
        if (pipe(fd) == -1) {
            fprintf(stderr, "Could not create pipe! (%s)\n", strerror(errno));
            goto failure;
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(fd[0]);
            double result = integrate(f, a + i * px, a + (i + 1) * px, dx);
            char buffer[BUFFER_SIZE];
            int size = sprintf(buffer, "%f", result);
            if (size < 0) {
                fprintf(stderr, "Double to String conversion error!\n");
                goto failure;
            }
            if (write(fd[1], buffer, size + 1) == -1) {
                fprintf(stderr, "Could not write to pipe! (%s)\n", strerror(errno));
                goto failure;
            }
            goto success;
        } else {
            close(fd[1]);
            pipes[i] = fd[0];
        }
    }

    while (wait(NULL) > 0);

    double result = 0;
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < n; ++i) {
        if (read(pipes[i], buffer, BUFFER_SIZE) == -1) {
            fprintf(stderr, "Could not read pipe! (%s)\n", strerror(errno));
            goto failure;
        }
        if (close(pipes[i]) == -1) {
            fprintf(stderr, "Could not close pipe (%s)\n", strerror(errno));
            goto failure;
        }
        result += strtod(buffer, NULL);
    }

    gettimeofday(&tEnd, NULL);
    timersub(&tEnd, &tStart, &tElapsed);
    printf("result=%f;sec=%ld,usec=%ld\n", result, tElapsed.tv_sec, tElapsed.tv_usec);

    success:
    fflush(NULL);
    free(pipes);
    return EXIT_SUCCESS;

    failure:
    fflush(NULL);
    free(pipes);
    return EXIT_FAILURE;
}
