#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BARBERS_COUNT 3
#define SEATS_COUNT 2
#define QUEUE_SIZE 2
#define HAIRCUTS_COUNT 6
#define NEW_CLIENT_PERIOD 1

typedef struct Salon {
    int queue[QUEUE_SIZE];
    int seats[SEATS_COUNT];
    int customerID;
} Salon;

Salon *salon;
int shmid;
sem_t *mutex;
sem_t *semCustomers;
sem_t *semBarbers;
sem_t *semSeats;

void customer() {
    sem_wait(mutex);
    int currentCustomerID = salon->customerID++;
    sem_post(mutex);

    sem_wait(mutex);

    bool foundSeat = false;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (salon->queue[i] == -1) {
            salon->queue[i] = currentCustomerID;
            printf("Klient %d zajmuje miejsce w poczekalni.\n", currentCustomerID);

            sem_post(semCustomers);
            foundSeat = true;
            break;
        }
    }

    if (!foundSeat && sem_trywait(semSeats) == -1) {
        printf("Klient %d opuszcza salon, brak miejsca w poczekalni.\n", currentCustomerID);
        sem_post(mutex);
        return;
    }
    sem_post(mutex);
    sem_wait(semSeats);
}

void barber(int barberID) {
    while (true) {
        sem_wait(semCustomers);
        sem_wait(mutex);

        int currentCustomerID = -1;
        for (int i = 0; i < QUEUE_SIZE; i++) {
            if (salon->queue[i] != -1) {
                currentCustomerID = salon->queue[i];
                salon->queue[i] = -1;
                break;
            }
        }
        if (currentCustomerID == -1) {
            sem_post(semBarbers);
            sem_post(mutex);
            continue;
        }

        printf("Fryzjer %d obsługuje klienta %d.\n", barberID, currentCustomerID);

        int occupiedSeat = -1;
        for (int i = 0; i < SEATS_COUNT; i++) {
            if (salon->seats[i] == -1) {
                salon->seats[i] = currentCustomerID;
                occupiedSeat = i;
                sem_post(semSeats);
                break;
            }
        }

        sem_post(mutex);
        sem_post(semBarbers);

        srand(time(NULL));
        int haircutTime = rand() % HAIRCUTS_COUNT + 1;
        sleep(haircutTime);

        printf("Fryzjer %d skończył obsługiwać klienta %d.\n", barberID, currentCustomerID);

        sem_wait(mutex);
        salon->seats[occupiedSeat] = -1;
        sem_post(mutex);
    }
}

int main() {
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

    mutex = sem_open("/mutex", O_CREAT, 0666, 1);
    semCustomers = sem_open("/semCustomers", O_CREAT, 0666, 0);
    semBarbers = sem_open("/semBarbers", O_CREAT, 0666, 0);
    semSeats = sem_open("/semSeats", O_CREAT, 0666, QUEUE_SIZE);

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

    return EXIT_SUCCESS;
}
