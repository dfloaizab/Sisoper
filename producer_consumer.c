#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Variables compartidas y estructuras de sincronización
int shared_data = 0;
int data_available = 0;  // Flag para indicar si hay datos disponibles
pthread_mutex_t mutex;
pthread_cond_t cond_var;

// Función del productor (Hilo 1)
void* producer(void* arg) {
    for (int i = 1; i <= 5; i++) {
        pthread_mutex_lock(&mutex);

        // Produce un valor y lo almacena en la variable compartida
        shared_data = i * 10;
        data_available = 1;  // Marca que los datos están disponibles
        printf("Productor: Generó el valor %d\n", shared_data);

        // Notifica al consumidor
        pthread_cond_signal(&cond_var);

        pthread_mutex_unlock(&mutex);
        sleep(1);  // Simula tiempo de producción
    }
    return NULL;
}

// Función del consumidor (Hilo 2)
void* consumer(void* arg) {
    for (int i = 1; i <= 5; i++) {
        pthread_mutex_lock(&mutex);

        // Espera hasta que el productor genere un valor
        while (data_available == 0) {
            pthread_cond_wait(&cond_var, &mutex);
        }

        // Consume el valor generado por el productor
        printf("Consumidor: Leyó el valor %d\n", shared_data);
        data_available = 0;  // Marca que los datos han sido consumidos

        pthread_mutex_unlock(&mutex);
        sleep(1);  // Simula tiempo de procesamiento
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    // Inicialización de mutex y variable de condición
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

    // Creación de los hilos
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Espera a que los hilos terminen
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Destruir mutex y variable de condición
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);

    return 0;
}
