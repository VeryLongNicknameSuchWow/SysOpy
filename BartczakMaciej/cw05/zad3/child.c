#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

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
    if (argc != 5) {
        fprintf(stderr, "Invalid argument count!\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    char *fifoPath = argv[1];
    double a = strtod(argv[2], NULL);
    double b = strtod(argv[3], NULL);
    double dx = strtod(argv[4], NULL);

    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "Could not open FIFO! (%s)\n", strerror(errno));
        goto failure;
    }

    double result = integrate(f, a, b, dx);
    if (write(fd, &result, sizeof(double)) == -1) {
        fprintf(stderr, "Could not write to pipe! (%s)\n", strerror(errno));
        goto failure;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Could not close pipe (%s)\n", strerror(errno));
        goto failure;
    }

    fflush(NULL);
    return EXIT_SUCCESS;

    failure:
    fflush(NULL);
    return EXIT_FAILURE;
}
