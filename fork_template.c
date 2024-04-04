#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

// Función para aumentar la memoria 
void aumentarMemoria(int n) {
    // Reserva n bloques de memoria 
    // COMPLETAR USANDO MALLOC
    if (memoria == NULL) {
        fprintf(stderr, "Error al reservar memoria\n");
        exit(EXIT_FAILURE);
    }
    //COMPLETAR: ESCRIBIR EN MEMORIA USANDO MEMSET
    printf("Memoria aumentada exitosamente\n");
}

int main() {
    // Variable para almacenar la cantidad de subprocesos a crear
    int num_subprocesos;

    printf("Ingrese el número de subprocesos a crear: ");
    scanf("%d", &num_subprocesos);

    // Bucle para crear el número especificado de subprocesos
    for (int i = 0; i < num_subprocesos; i++) {
        // COMPLETAR: Crear Subproceso
        pid_t pid = //

        if (pid < 0) {
            // Error al crear el subproceso hijo
            fprintf(stderr, "Error al crear el subproceso hijo\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            // Completar: pedir un número de bloques de memoria al usuario y reservar
            //llamando a la función implementada más arriba
            int n;
            //COMPLETAR:
            exit(EXIT_SUCCESS);
        }
    }

    // Esperamos a que todos los subprocesos hijos terminen
    for (int i = 0; i < num_subprocesos; i++) {
        wait(NULL);
    }

    return EXIT_SUCCESS;
}
