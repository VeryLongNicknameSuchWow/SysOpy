#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define REINDEER_TOTAL 9
#define ELVES_TOTAL 10
#define ELVES_REQUIRED 3
#define MAX_DELIVERIES 3

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_cond_t reindeerCond;
    pthread_cond_t elfCond;
    unsigned int elves, reindeer, deliveries;
    int elvesQueue[ELVES_REQUIRED];
    bool is_running;
} NorthPole;

NorthPole northPole;

void *reindeerRoutine(void *arg) {
    int reindeerID = (int) arg;

    while (true) {
        printf("Reindeer %d went on vacation \n", reindeerID);
        sleep(5 + (rand() % 6));

        pthread_mutex_lock(&northPole.mutex);
        if (!northPole.is_running) {
            pthread_mutex_unlock(&northPole.mutex);
            break;
        }
        northPole.reindeer++;
        printf("Reindeer %d came back from vacation, total reindeer waiting: %d\n", reindeerID, northPole.reindeer);
        if (northPole.reindeer == REINDEER_TOTAL) {
            printf("Reindeer %d wakes up Santa\n", reindeerID);
        }
        pthread_cond_signal(&northPole.condition);
        pthread_cond_wait(&northPole.reindeerCond, &northPole.mutex);
        pthread_mutex_unlock(&northPole.mutex);
    }

    return NULL;
}

void *elfRoutine(void *arg) {
    int elfID = (int) arg;

    while (true) {
        sleep(2 + (rand() % 4));

        pthread_mutex_lock(&northPole.mutex);
        if (!northPole.is_running) {
            pthread_mutex_unlock(&northPole.mutex);
            break;
        }
        if (northPole.elves < ELVES_REQUIRED) {
            northPole.elvesQueue[northPole.elves] = elfID;
            northPole.elves += 1;
            printf("Elf %d is waiting for santa, total elves waiting: %d\n", elfID, northPole.elves);
            if (northPole.elves == ELVES_REQUIRED) {
                printf("Elf %d wakes up santa\n", elfID);
            }
            pthread_cond_signal(&northPole.condition);
        } else {
            printf("Elf %d solved the problem on their own\n", elfID);
            pthread_mutex_unlock(&northPole.mutex);
            continue;
        }
        pthread_cond_wait(&northPole.elfCond, &northPole.mutex);
        pthread_mutex_unlock(&northPole.mutex);
    }

    return NULL;
}

int main(int argc, char **argv) {
    pthread_mutex_init(&(northPole.mutex), NULL);
    pthread_cond_init(&(northPole.condition), NULL);
    pthread_cond_init(&(northPole.reindeerCond), NULL);
    pthread_cond_init(&(northPole.elfCond), NULL);
    northPole.reindeer = 0;
    northPole.elves = 0;
    northPole.deliveries = 0;
    northPole.is_running = true;

    pthread_t reindeer_ids[REINDEER_TOTAL];
    for (int i = 0; i < REINDEER_TOTAL; i++) {
        pthread_create(&reindeer_ids[i], NULL, reindeerRoutine, (void *) i);
    }

    pthread_t elf_ids[ELVES_TOTAL];
    for (int i = 0; i < ELVES_TOTAL; i++) {
        pthread_create(&elf_ids[i], NULL, elfRoutine, (void *) i);
    }

    while (northPole.is_running) {
        pthread_mutex_lock(&northPole.mutex);
        while (northPole.reindeer < REINDEER_TOTAL && northPole.elves < ELVES_REQUIRED) {
            pthread_cond_wait(&northPole.condition, &northPole.mutex);
        }
        if (northPole.reindeer == REINDEER_TOTAL) {
            printf("Santa is delivering gifts\n");
            sleep(2 + rand() % 3);
            northPole.reindeer = 0;
            northPole.deliveries += 1;
            if (northPole.deliveries == MAX_DELIVERIES) {
                printf("Santa finished all deliveries!\n");
                northPole.is_running = false;
                pthread_cond_broadcast(&northPole.reindeerCond);
                pthread_cond_broadcast(&northPole.elfCond);
                pthread_mutex_unlock(&northPole.mutex);
                break;
            }
            pthread_cond_broadcast(&northPole.reindeerCond);
        } else if (northPole.elves == ELVES_REQUIRED) {
            printf("Santa is helping elves: ");
            for (int i = 0; i < ELVES_REQUIRED; i++) {
                printf("%d ", northPole.elvesQueue[i]);
            }
            printf("\n");
            sleep(1 + rand() % 2);
            northPole.elves = 0;
            pthread_cond_broadcast(&northPole.elfCond);
        }
        pthread_mutex_unlock(&northPole.mutex);
    }

    for (int i = 0; i < REINDEER_TOTAL; i++) {
        pthread_join(reindeer_ids[i], NULL);
    }

    for (int i = 0; i < ELVES_TOTAL; i++) {
        pthread_join(elf_ids[i], NULL);
    }

    pthread_mutex_destroy(&northPole.mutex);
    pthread_cond_destroy(&northPole.condition);
    pthread_cond_destroy(&northPole.reindeerCond);
    pthread_cond_destroy(&northPole.elfCond);

    return EXIT_SUCCESS;
}
