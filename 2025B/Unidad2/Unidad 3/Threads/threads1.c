#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5 // Tamaño máximo del búfer compartido

// Buffer circular compartido entre productor y consumidor
int buffer[BUFFER_SIZE];
int count = 0; // Número de elementos actualmente en el buffer

// Mecanismos de sincronización
pthread_mutex_t mutex;             // Excluye acceso simultáneo al buffer
pthread_cond_t cond_productor;     // Señala al productor cuando hay espacio disponible
pthread_cond_t cond_consumidor;    // Señala al consumidor cuando hay datos disponibles

// --------------------------------------------------------------------
// FUNCIÓN DEL PRODUCTOR
// --------------------------------------------------------------------
void* productor(void* arg) {
    int i = 0;
    while (1) {
        pthread_mutex_lock(&mutex); // Bloquea el acceso al buffer

        // Si el buffer está lleno, el productor espera
        while (count == BUFFER_SIZE) {
            printf("[Productor] Esperando, buffer lleno...\n");
            pthread_cond_wait(&cond_productor, &mutex);
        }

        // Produce un nuevo ítem
        buffer[count] = i;
        printf("[Productor] Produjo: %d (count = %d)\n", i, count + 1);
        count++;
        i++;

        // Despierta al consumidor (si estaba esperando)
        pthread_cond_signal(&cond_consumidor);

        pthread_mutex_unlock(&mutex); // Libera el buffer

        sleep(1); // Simula tiempo de producción
    }
    return NULL;
}

// --------------------------------------------------------------------
// FUNCIÓN DEL CONSUMIDOR
// --------------------------------------------------------------------
void* consumidor(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex); // Bloquea el acceso al buffer

        // Si el buffer está vacío, el consumidor espera
        while (count == 0) {
            printf("[Consumidor] Esperando, buffer vacío...\n");
            pthread_cond_wait(&cond_consumidor, &mutex);
        }

        // Consume un ítem (extrae el último elemento disponible)
        count--;
        int item = buffer[count];
        printf("[Consumidor] Consumió: %d (count = %d)\n", item, count);

        // Despierta al productor (si estaba esperando)
        pthread_cond_signal(&cond_productor);

        pthread_mutex_unlock(&mutex); // Libera el buffer

        sleep(2); // Simula tiempo de consumo
    }
    return NULL;
}

// --------------------------------------------------------------------
// FUNCIÓN PRINCIPAL
// --------------------------------------------------------------------
int main() {
    pthread_t tprod, tcons; // Identificadores de hilos

    // Inicializar los mecanismos de sincronización
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_productor, NULL);
    pthread_cond_init(&cond_consumidor, NULL);

    // Crear los hilos productor y consumidor
    pthread_create(&tprod, NULL, productor, NULL);
    pthread_create(&tcons, NULL, consumidor, NULL);

    // Esperar a que ambos hilos terminen (en este caso nunca lo harán)
    pthread_join(tprod, NULL);
    pthread_join(tcons, NULL);

    // Liberar recursos del sistema
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_productor);
    pthread_cond_destroy(&cond_consumidor);

    return 0;
}
