#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIGNAL SIGUSR1
bool isManualChild = false;
char *command = "";
char *instruction = "";

int execChild() {
    printf("PID %d:\texiting successfully\t(replaced with exec)\n", getpid());
    fflush(stdout);
    return execl(command, command, instruction, "child", NULL);
}

void handleSignal(int signum) {
    printf("PID %d:\thandling signal\n", getpid());
    fflush(stdout);
}

void raiseSignal() {
    printf("PID %d:\traising signal\n", getpid());
    fflush(stdout);
    raise(SIGNAL);
}

void setIgnoreSignal() {
    printf("PID %d:\tenabling ignore\n", getpid());
    fflush(stdout);
    signal(SIGNAL, SIG_IGN);
}

void setMaskSignal() {
    printf("PID %d:\tenabling mask\n", getpid());
    fflush(stdout);
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGNAL);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
}

void setHandleSignal() {
    printf("PID %d:\tenabling handler\n", getpid());
    fflush(stdout);
    signal(SIGNAL, handleSignal);
}

void checkPendingSignal() {
    sigset_t sigset;
    sigpending(&sigset);
    if (sigismember(&sigset, SIGNAL)) {
        printf("PID %d:\tsignal is pending\n", getpid());
        fflush(stdout);
    } else {
        printf("PID %d:\tsignal is not pending\n", getpid());
        fflush(stdout);
    }
}

void ignoreTask() {
    if (!isManualChild) {
        setIgnoreSignal();
        raiseSignal();
        if (fork() == 0) {
            printf("PID %d:\tis a child process\t\t(fork)\n", getpid());
            fflush(stdout);
            raiseSignal();
        } else {
            wait(NULL);
            execChild();
        }
    } else {
        raiseSignal();
    }
}

void handlerTask() {
    if (!isManualChild) {
        setHandleSignal();
        raiseSignal();
        if (fork() == 0) {
            printf("PID %d:\tis a child process\t\t(fork)\n", getpid());
            fflush(stdout);
            raiseSignal();
        } else {
            wait(NULL);
            execChild();
        }
    } else {
        raiseSignal();
    }
}

void maskTask() {
    if (!isManualChild) {
        setHandleSignal();
        setMaskSignal();
        raiseSignal();
        checkPendingSignal();
        if (fork() == 0) {
            printf("PID %d:\tis a child process\t\t(fork)\n", getpid());
            fflush(stdout);
            raiseSignal();
            checkPendingSignal();
        } else {
            wait(NULL);
            execChild();
        }
    } else {
        raiseSignal();
        checkPendingSignal();
    }
}

void pendingTask() {
    if (!isManualChild) {
        setHandleSignal();
        setMaskSignal();
        raiseSignal();
        checkPendingSignal();
        if (fork() == 0) {
            printf("PID %d:\tis a child process\t\t(fork)\n", getpid());
            fflush(stdout);
            checkPendingSignal();
        } else {
            wait(NULL);
            execChild();
        }
    } else {
        checkPendingSignal();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        return EXIT_FAILURE;
    }

    if (argc == 3) {
        if (strcmp(argv[2], "child") == 0) {
            isManualChild = true;
        } else {
            return EXIT_FAILURE;
        }
    }

    if (isManualChild) {
        printf("PID %d:\tis a child process\t\t(exec)\n", getpid());
        fflush(stdout);
    } else {
        printf("PID %d:\tis the main process\n", getpid());
        fflush(stdout);
    }

    command = argv[0];
    instruction = argv[1];
    if (strcmp(instruction, "ignore") == 0) {
        ignoreTask();
    } else if (strcmp(instruction, "handler") == 0) {
        handlerTask();
    } else if (strcmp(instruction, "mask") == 0) {
        maskTask();
    } else if (strcmp(instruction, "pending") == 0) {
        pendingTask();
    } else {
        return EXIT_FAILURE;
    }

    wait(NULL);
    if (isManualChild) {
        printf("PID %d:\texiting successfully\t(exec)\n", getpid());
        fflush(stdout);
    } else {
        printf("PID %d:\texiting successfully\t(fork)\n", getpid());
        fflush(stdout);
    }

    return EXIT_SUCCESS;
}