#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

sem_t sem;

void* tarea(void* arg) {
    sem_wait(&sem);
    printf("Acceso exclusivo por hilo.\n");
    sem_post(&sem);
    return NULL;
}

int main() {
    pthread_t hilos[2];
    sem_init(&sem, 0, 1);

    pthread_create(&hilos[0], NULL, tarea, NULL);
    pthread_create(&hilos[1], NULL, tarea, NULL);

    pthread_join(hilos[0], NULL);
    pthread_join(hilos[1], NULL);
    sem_destroy(&sem);
    return 0;
}
