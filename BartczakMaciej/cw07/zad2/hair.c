#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>

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
int shmfd;
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
    shmfd = shm_open("/salon_shm", O_CREAT | O_RDWR, 0666);
    if (shmfd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shmfd, sizeof(Salon)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    salon = (Salon *) mmap(NULL, sizeof(Salon), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (salon == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    mutex = sem_open("/salon_mutex", O_CREAT, 0666, 1);
    semCustomers = sem_open("/salon_customers", O_CREAT, 0666, 0);
    semBarbers = sem_open("/salon_barbers", O_CREAT, 0666, 0);
    semSeats = sem_open("/salon_seats", O_CREAT, 0666, QUEUE_SIZE);

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

    return EXIT_SUCCESS;
}
