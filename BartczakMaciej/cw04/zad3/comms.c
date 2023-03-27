#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define SIGNAL SIGUSR1

typedef enum {
    PRINT_NUMBERS = 1,
    PRINT_TIME_ONCE = 2,
    PRINT_TASK_COUNTER = 3,
    PRINT_TIME_LOOP = 4,
    EXIT = 5,
} CatcherTask;

void commonAwaitSignal() {
    sigset_t response;
    sigemptyset(&response);
    sigsuspend(&response);
}

void sendConfirmation(pid_t pid) {
    printf("[CATCHER] sent confirmation\n");
    fflush(stdout);
    kill(pid, SIGNAL);
}

void receiveConfirmation(int sig) {
    printf("[SENDER] received confirmation\n");
    fflush(stdout);
}

void sender(pid_t catcherPID, const CatcherTask tasks[], int n) {
    signal(SIGNAL, receiveConfirmation);
    printf("[SENDER] (PID=%d) is ready\n", getpid());
    fflush(stdout);

    union sigval sv;
    for (int i = 0; i < n; ++i) {
        sv.sival_int = tasks[i];
        printf("[SENDER] sending signal %d to PID=%d\n", sv.sival_int, catcherPID);
        fflush(stdout);
        sigqueue(catcherPID, SIGNAL, sv);
        commonAwaitSignal();
    }
    printf("[SENDER] exiting\n");
    fflush(stdout);
}

void catcherHandler(int sig, siginfo_t *siginfo, void *context) {
    static int counter = 0;
    counter++;

    pid_t sender = siginfo->si_pid;
    int status = siginfo->si_status;
    printf("[CATCHER] received %d\n", status);
    fflush(stdout);

    CatcherTask task = (CatcherTask) status;
    switch (task) {
        case PRINT_NUMBERS:
            printf("[CATCHER] ");
            printf("1, ");
            printf("10, ");
            printf("11, ");
//            for (int i = 1; i < 100; ++i) {
//                printf("%d, ", i);
//            }
            printf("100\n");
            fflush(stdout);
            break;
        case PRINT_TIME_ONCE:
            printf("[CATCHER] Epoch: %ld (once)\n", time(0));
            fflush(stdout);
            break;
        case PRINT_TASK_COUNTER:
            printf("[CATCHER] Counter: %d\n", counter);
            fflush(stdout);
            break;
        case PRINT_TIME_LOOP:
            sendConfirmation(sender);
            while (1) {
                printf("[CATCHER] Epoch: %ld (loop)\n", time(0));
                fflush(stdout);
                sleep(1);
            }
        case EXIT:
            printf("[CATCHER] Exiting\n");
            fflush(stdout);
            sendConfirmation(sender);
            exit(0);
        default:
            break;
    }

    sendConfirmation(sender);
}

void catcher() {
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = catcherHandler;
    action.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigaction(SIGNAL, &action, NULL);

    printf("[CATCHER] (PID=%d) is ready\n", getpid());
    fflush(stdout);

    while (1) {
        commonAwaitSignal();
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        catcher();
        return EXIT_SUCCESS;
    }

    int tasksSize = argc - 2;
    if (tasksSize <= 0) {
        return EXIT_FAILURE;
    }

    CatcherTask tasks[tasksSize];
    char **textTasks = argv + 2;
    for (int i = 0; i < tasksSize; ++i) {
        tasks[i] = (CatcherTask) strtol(textTasks[i], NULL, 10);
    }

    //dodatkowa opcja uruchamiająca catcher w procesie, bez ręcznego kopiowania PID
    if (strcmp(argv[1], "bundled") == 0) {
        pid_t catcherPID = getpid();
        pid_t pid = fork();
        if (pid == 0) {
            sender(catcherPID, tasks, tasksSize);
        } else {
            catcher();
        }

        return EXIT_SUCCESS;
    }

    pid_t catcherPID = (pid_t) strtol(argv[1], NULL, 10);
    sender(catcherPID, tasks, tasksSize);

    return EXIT_SUCCESS;
}