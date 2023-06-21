#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define L_SLOW 8
#define NUM_THREADS 20

pthread_mutex_t mutex01 = PTHREAD_MUTEX_INITIALIZER;

char slownik[L_SLOW][10] = {"alfa", "bravo", "charlie", "delta", "echo", "foxtrot", "golf", "hotel"};
int NR = 0;

void *fun_watka(void *parametr) {
    //zajmij mutex 'mutex01'
    pthread_mutex_lock(&mutex01);

    printf("%s ", slownik[NR++]);
    fflush(stdout);
    if (NR >= L_SLOW) {
        NR = 0;
    }

    //zwolnij mutex 'mutex01'
    pthread_mutex_unlock(&mutex01);

    sleep(1);
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int threadArgs[NUM_THREADS];
    int resultCode;
    int i;

    //Utworz 20 watkow realizujacych funkcje 'fun_watka'
    for (i = 0; i < NUM_THREADS; i++) {
        threadArgs[i] = i;
        resultCode = pthread_create(&threads[i], NULL, fun_watka, &threadArgs[i]);
        if (resultCode != 0) {
            printf("blad pthread_create\n");
            return EXIT_FAILURE;
        }
    }

    //poczekaj na zakonczenie wszystkich watkow
    for (i = 0; i < NUM_THREADS; i++) {
        resultCode = pthread_join(threads[i], NULL);
        if (resultCode != 0) {
            printf("blad pthread_join\n");
            return EXIT_FAILURE;
        }
    }

    printf("\n");
    return EXIT_SUCCESS;
}
