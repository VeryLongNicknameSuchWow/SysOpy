#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define SIGNAL SIGUSR1
#define CALL_LIMIT 5

void sigaction_SIGINFO(int sig, siginfo_t *siginfo, void *context) {
    printf("SIGINFO: Signal number: %d\n", siginfo->si_signo);
    printf("SIGINFO: Sender PID:    %d\n", siginfo->si_pid);
    // 3 różne informacje
    printf("SIGINFO: Sender UID:    %d\n", siginfo->si_uid);
    printf("SIGINFO: Errno:         %d\n", siginfo->si_errno);
    printf("SIGINFO: Exit value:    %d\n", siginfo->si_status);
}

void handler_NODEFER(int sig) {
    static int counter = 0;
    printf("NODEFER: handler call counter:  %d\n", ++counter);

    if (counter < CALL_LIMIT) {
        printf("NODEFER: raising signal inside the handler\n");
        raise(SIGNAL);
    } else {
        counter = 0;
        printf("NODEFER: call limit reached\n");
    }
}

void handler_RESETHAND(int sig) {
    printf("RESETHAND: Signal number: %d\n", sig);
}

void raiseSignal() {
    printf("Raising signal...\n");
    raise(SIGNAL);
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        return EXIT_FAILURE;
    }

    struct sigaction action;
    sigemptyset(&action.sa_mask);

    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = sigaction_SIGINFO;
    sigaction(SIGNAL, &action, NULL);
    raiseSignal();

    action.sa_flags = SA_NODEFER;
    action.sa_handler = handler_NODEFER;
    sigaction(SIGNAL, &action, NULL);
    raiseSignal();

    action.sa_flags = SA_RESETHAND;
    action.sa_handler = handler_RESETHAND;
    sigaction(SIGNAL, &action, NULL);
    raiseSignal();
    raiseSignal();

    return EXIT_SUCCESS;
}