#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>


#define BARBERS_COUNT 3
#define SEATS_COUNT 2
#define QUEUE_SIZE 2
#define HAIRCUTS_COUNT 6
#define NEW_CLIENT_PERIOD 1

typedef struct Salon {
    int queue[QUEUE_SIZE];
    int seats[SEATS_COUNT];
    int customerID;
    sem_t mutex;
    sem_t semCustomers;
    sem_t semBarbers;
    sem_t semSeats;
} Salon;

Salon *salon;
int shmid;

void exitChild() {
    sem_post(&salon->mutex);
    sem_post(&salon->semCustomers);
    sem_post(&salon->semBarbers);
    sem_post(&salon->semSeats);
}

void stopSimulation() {
    sem_destroy(&salon->mutex);
    sem_destroy(&salon->semCustomers);
    sem_destroy(&salon->semBarbers);
    sem_destroy(&salon->semSeats);

    if (shmdt(salon) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}

void customer() {
    sem_wait(&salon->mutex);
    int currentCustomerID = salon->customerID++;
    sem_post(&salon->mutex);

    sem_wait(&salon->mutex);

    bool foundSeat = false;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (salon->queue[i] == -1) {
            salon->queue[i] = currentCustomerID;
            printf("Klient %d zajmuje miejsce w poczekalni.\n", currentCustomerID);

            sem_post(&salon->semCustomers);
            foundSeat = true;
            break;
        }
    }

    if (!foundSeat && sem_trywait(&salon->semSeats) == -1) {
        printf("Klient %d opuszcza salon, brak miejsca w poczekalni.\n", currentCustomerID);
        sem_post(&salon->mutex);
        return;
    }
    sem_post(&salon->mutex);
    sem_wait(&salon->semSeats);
}

void barber(int barberID) {
    while (true) {
        sem_wait(&salon->semCustomers);
        sem_wait(&salon->mutex);

        int currentCustomerID = -1;
        for (int i = 0; i < QUEUE_SIZE; i++) {
            if (salon->queue[i] != -1) {
                currentCustomerID = salon->queue[i];
                salon->queue[i] = -1;
                break;
            }
        }
        if (currentCustomerID == -1) {
            sem_post(&salon->semBarbers);
            sem_post(&salon->mutex);
            continue;
        }

        printf("Fryzjer %d obsługuje klienta %d.\n", barberID, currentCustomerID);

        int occupiedSeat = -1;
        for (int i = 0; i < SEATS_COUNT; i++) {
            if (salon->seats[i] == -1) {
                salon->seats[i] = currentCustomerID;
                occupiedSeat = i;
                sem_post(&salon->semSeats);
                break;
            }
        }

        sem_post(&salon->mutex);
        sem_post(&salon->semBarbers);

        srand(time(NULL));
        int haircutTime = rand() % HAIRCUTS_COUNT + 1;
        sleep(haircutTime);

        printf("Fryzjer %d skończył obsługiwać klienta %d.\n", barberID, currentCustomerID);

        sem_wait(&salon->mutex);
        salon->seats[occupiedSeat] = -1;
        sem_post(&salon->mutex);
    }
}

int main() {
    signal(SIGINT, exitChild);

    shmid = shmget(IPC_PRIVATE, sizeof(Salon), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    salon = (Salon *) shmat(shmid, NULL, 0);
    if (salon == (Salon *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    sem_init(&salon->mutex, true, 1);
    sem_init(&salon->semCustomers, true, 0);
    sem_init(&salon->semBarbers, true, 0);
    sem_init(&salon->semSeats, true, QUEUE_SIZE);

    for (int i = 0; i < QUEUE_SIZE; i++) {
        salon->queue[i] = -1;
    }

    for (int i = 0; i < SEATS_COUNT; i++) {
        salon->seats[i] = -1;
    }

    pid_t pid;

    for (int i = 0; i < BARBERS_COUNT; i++) {
        pid = fork();
        if (pid == 0) {
            barber(i);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    signal(SIGINT, stopSimulation);
    salon->customerID = 1;

    while (true) {
        pid = fork();
        if (pid == 0) {
            customer();
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        sleep(NEW_CLIENT_PERIOD);
    }
}
