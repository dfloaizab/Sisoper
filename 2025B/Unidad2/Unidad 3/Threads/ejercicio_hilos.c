/*
    SISTEMAS OPERATIVOS, 2025B
    CONCURRENCIA Y SINCRONIZACIÓN DE HILOS
    Ejercicio: Sincronización de hilos en C (Productor/Consumidor simplificado)
    Descripción:
    Este programa crea dos hilos: uno incrementa una variable compartida y otro la decrementa.
    El objetivo es evitar condiciones de carrera usando mecanismos de sincronización.
    Complete las partes marcadas con "TODO" para implementar un control adecuado.

    Temas que se practican:
    - Creación de hilos (pthread_create)
    - Espera por hilos (pthread_join)
    - Exclusión mutua (pthread_mutex_t)
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ITERACIONES 1000000

// Variable global compartida entre los hilos
int contador = 0;

// TODO: declarar aquí el mutex para sincronizar el acceso al contador
// Ejemplo: pthread_mutex_t lock;

////////////////////////////////////////////////////////////
// Función: incrementar
// Descripción: Incrementa la variable global contador ITERACIONES veces
////////////////////////////////////////////////////////////
void* incrementar(void* arg) {
    for (int i = 0; i < ITERACIONES; i++) {

        // TODO: bloquear el acceso al contador antes de modificarlo
        // pthread_mutex_lock(...);

        contador++;

        // TODO: liberar el acceso al contador después de modificarlo
        // pthread_mutex_unlock(...);
    }
    return NULL;
}

////////////////////////////////////////////////////////////
// Función: decrementar
// Descripción: Decrementa la variable global contador ITERACIONES veces
////////////////////////////////////////////////////////////
void* decrementar(void* arg) {
    for (int i = 0; i < ITERACIONES; i++) {

        // TODO: bloquear el acceso al contador antes de modificarlo
        // pthread_mutex_lock(...);

        contador--;

        // TODO: liberar el acceso al contador después de modificarlo
        // pthread_mutex_unlock(...);
    }
    return NULL;
}

////////////////////////////////////////////////////////////
// Función principal
////////////////////////////////////////////////////////////
int main() {
    pthread_t hilo1, hilo2;

    // TODO: inicializar el mutex
    // pthread_mutex_init(&lock, NULL);

    // Crear los dos hilos
    pthread_create(&hilo1, NULL, incrementar, NULL);
    pthread_create(&hilo2, NULL, decrementar, NULL);

    // Esperar a que terminen
    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);

    // Imprimir resultado final
    printf("Valor final del contador: %d\n", contador);

    // TODO: destruir el mutex
    // pthread_mutex_destroy(&lock);

    return 0;
}
