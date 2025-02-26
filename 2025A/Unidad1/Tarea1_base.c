#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Variables globales (Segmento de Datos)
int global_var = 42;
static int static_global_var = 100;

// Función para mostrar direcciones de memoria de distintos segmentos
void mostrar_segmentos() {
    int local_var = 10; // Variable local (Pila)
    static int static_local_var = 20; // Variable estática local (Datos)
    int *heap_var = (int*) malloc(sizeof(int)); // Variable en Heap

    printf("\n--- Direcciones de memoria ---\n");
    printf("Variable global: %p\n", (void*)&global_var);
    printf("Variable estática global: %p\n", (void*)&static_global_var);
    printf("Variable local: %p\n", (void*)&local_var);
    printf("Variable estática local: %p\n", (void*)&static_local_var);
    printf("Variable en Heap: %p\n", (void*)heap_var);

    free(heap_var); // Liberar memoria asignada en Heap
}

// Función para mostrar el consumo de memoria del proceso usando /proc/self/status
void mostrar_consumo_memoria() {
    char comando[50];
    snprintf(comando, sizeof(comando), "cat /proc/%d/status | grep -E 'VmSize|VmRSS'", getpid());
    printf("\n--- Consumo de Memoria ---\n");
    system(comando);
}

// Función para asignar memoria dinámica
void asignar_memoria_dinamica(size_t size) {
    int *arr = (int*) malloc(size * sizeof(int));
    if (arr == NULL) {
        printf("Error al asignar memoria.\n");
        return;
    }

    // Llenar el arreglo con datos
    for (size_t i = 0; i < size; i++) {
        arr[i] = i;
    }

    printf("Se asignaron %zu enteros en memoria dinámica.\n", size);
    mostrar_consumo_memoria();

    // Esperar entrada del usuario antes de liberar memoria
    printf("Presiona ENTER para liberar la memoria...");
    getchar();
    free(arr);
    printf("Memoria liberada.\n");
    mostrar_consumo_memoria();
}

// Menú interactivo
void menu() {
    int opcion;
    size_t size;

    do {
        printf("\n--- Menú de Gestión de Memoria ---\n");
        printf("1. Mostrar direcciones de memoria\n");
        printf("2. Mostrar consumo de memoria\n");
        printf("3. Asignar memoria dinámica\n");
        printf("4. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);
        getchar(); // Limpiar buffer de entrada

        switch (opcion) {
            case 1:
                mostrar_segmentos();
                break;
            case 2:
                mostrar_consumo_memoria();
                break;
            case 3:
                printf("Ingrese la cantidad de enteros a asignar en memoria: ");
                scanf("%zu", &size);
                getchar(); // Limpiar buffer de entrada
                asignar_memoria_dinamica(size);
                break;
            case 4:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opción no válida.\n");
        }
    } while (opcion != 4);
}

int main() {
    menu();
    return 0;
}
