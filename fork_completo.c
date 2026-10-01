#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define TAM_BLOQUE (1024 * 1024)   // 1 bloque = 1 MB
#define TIEMPO_ESPERA 60           // segundos que el hijo mantiene la memoria

// Función para aumentar la memoria
void aumentarMemoria(int n) {
    size_t total = (size_t)n * TAM_BLOQUE;

    // Reserva n bloques de memoria
    char *memoria = malloc(total);
    if (memoria == NULL) {
        fprintf(stderr, "Error al reservar memoria\n");
        exit(EXIT_FAILURE);
    }

    // Escribir en memoria: obliga al kernel a asignar páginas físicas reales
    memset(memoria, 'A', total);
    printf("[PID %d] Memoria aumentada exitosamente: %d MB\n", getpid(), n);

    // Mantener el proceso vivo para poder observarlo con ps, top, etc.
    sleep(TIEMPO_ESPERA);

    free(memoria);
}

int main() {
    // Variable para almacenar la cantidad de subprocesos a crear
    int num_subprocesos;

    printf("PID del proceso padre: %d\n", getpid());
    printf("Ingrese el número de subprocesos a crear: ");
    fflush(stdout);
    scanf("%d", &num_subprocesos);

    // Bucle para crear el número especificado de subprocesos
    for (int i = 0; i < num_subprocesos; i++) {
        // Crear Subproceso
        pid_t pid = fork();

        if (pid < 0) {
            // Error al crear el subproceso hijo
            fprintf(stderr, "Error al crear el subproceso hijo\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Proceso hijo
            printf("[Hijo %d] PID=%d, PPID=%d\n", i + 1, getpid(), getppid());
            int n;
            printf("[PID %d] Número de bloques de memoria (1 bloque = 1 MB): ", getpid());
            fflush(stdout);
            scanf("%d", &n);
            aumentarMemoria(n);
            exit(EXIT_SUCCESS);
        }
    }

    // Esperamos a que todos los subprocesos hijos terminen
    for (int i = 0; i < num_subprocesos; i++) {
        wait(NULL);
    }

    return EXIT_SUCCESS;
}
