#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define L_SLOW 8

pthread_mutex_t mutex01 = PTHREAD_MUTEX_INITIALIZER;

char slownik[L_SLOW][10] = {"alfa", "bravo", "charlie", "delta", "echo", "foxtrot", "golf", "hotel"};
int NR = 0;

void *fun_watka(void *parametr) {
    //zajmij mutex 'mutex01'
    //...

    printf("%s ", slownik[NR++]);
    fflush(stdout);
    if (NR >= L_SLOW) NR = 0;
    //zwolnij mutex 'mutex01'
    //...

    sleep(1);
}

int main(void) {
    int i;
    //Utworz 20 watkow realizujacych funkcje 'fun_watka'
    //...

    //poczekaj na zakonczenie wszystkich watkow
    //...

    printf("\n");

}
