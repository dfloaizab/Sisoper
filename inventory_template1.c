#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define INVENTORY_SIZE 1    // Número de tipos de artículos en el inventario
#define NUM_OPERATIONS 10   // Número de operaciones por cada hilo

int inventory[INVENTORY_SIZE] = {100}; // Inventario inicial
pthread_mutex_t inventory_mutex;         // Mutex para sincronizar el acceso al inventario

// Función para agregar artículos al inventario
void* add_inventory(void* arg) {
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        pthread_mutex_lock(&inventory_mutex); // Bloquear el mutex

        int cantidad = rand() % 20 + 1; // Cantidad aleatoria entre 1 y 20
        inventory[0] += cantidad;
        printf("Hilo Agregar: Agregados %d artículos. Stock actual: %d\n", cantidad, inventory[0]);

        pthread_mutex_unlock(&inventory_mutex); // Desbloquear el mutex

        sleep(1); // Simular tiempo de procesamiento
    }
    pthread_exit(NULL);
}

// Función para retirar artículos del inventario
void* remove_inventory(void* arg) {
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        pthread_mutex_lock(&inventory_mutex); // Bloquear el mutex

        int cantidad = rand() % 20 + 1; // Cantidad aleatoria entre 1 y 20
        if (inventory[0] >= cantidad) {
            inventory[0] -= cantidad;
            printf("Hilo Retirar: Retirados %d artículos. Stock actual: %d\n", cantidad, inventory[0]);
        } else {
            printf("Hilo Retirar: No hay suficiente stock para retirar %d artículos. Stock actual: %d\n", cantidad, inventory[0]);
        }

        pthread_mutex_unlock(&inventory_mutex); // Desbloquear el mutex

        sleep(1); // Simular tiempo de procesamiento
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t hilo_agregar, hilo_retirar;

    // Inicializar el mutex
    if (pthread_mutex_init(&inventory_mutex, NULL) != 0) {
        printf("Error al inicializar el mutex\n");
        return 1;
    }

    // Crear los hilos
    if (pthread_create(&hilo_agregar, NULL, add_inventory, NULL) != 0) {
        printf("Error al crear el hilo de agregar\n");
        return 1;
    }

    if (pthread_create(&hilo_retirar, NULL, remove_inventory, NULL) != 0) {
        printf("Error al crear el hilo de retirar\n");
        return 1;
    }

    // Esperar a que los hilos terminen
    pthread_join(hilo_agregar, NULL);
    pthread_join(hilo_retirar, NULL);

    // Destruir el mutex
    pthread_mutex_destroy(&inventory_mutex);

    // Mostrar el inventario final
    printf("Inventario final: %d artículos\n", inventory[0]);

    return 0;
}
