#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

// --- Función de tarea CPU-bound ---
void cpu_task(int n, const char *name) {
    for (int i = 0; i < n; i++) {
        if (i % (n/5 + 1) == 0) {
            printf("[%s] paso %d (pid=%d)\n", name, i, getpid());
            //boni: sleep aleatorio
            usleep(50000); // pequeña pausa simulada
        }
    }
    printf("[%s] terminado (pid=%d)\n", name, getpid());
}

int main() {
    int n; 
    printf("Ingrese cantidad de procesos a crear: ");
    scanf("%d", &n);

    // (1) TODO: reservar memoria dinámica para un arreglo de PIDs (pid_t) y otro de cargas (int)
    // Ejemplo: pid_t *pids = ... ; int *cargas = ...;
    

    // (2) TODO: inicializar las cargas en el arreglo "cargas"
    // Idea: que cada proceso tenga una carga mayor que el anterior
    // Sugerencia: cargas[i] = (i+1) * 5000000;
    

    // --- Crear procesos hijos ---
    for (int i = 0; i < n; i++) {
        // (3) TODO: llamar a fork() y guardar el PID en pids[i]
        

        if (/* TODO: condición para saber si es hijo */) {
            // (4) TODO: usar nice() para variar prioridades
            // Sugerencia: si i % 2 == 0 => nice(5); else => nice(0);
            // el "niceness" de un proceso es una métrica que se calcula de acuerdo a la prioridad:
            // es un valor de -20 a 19
            // ver: https://askubuntu.com/questions/656771/process-niceness-vs-priority

            // Construir nombre de proceso
            char nombre[32];
            sprintf(nombre, "Proc-%d", i+1);

            // Ejecutar tarea
            cpu_task(cargas[i], nombre);
            exit(0); // hijo termina aquí
        }
    }

    // --- Esperar a todos los hijos ---
    for (int i = 0; i < n; i++) {
        // (5) TODO: usar waitpid() para esperar a pids[i]
        
    }

    // Liberar memoria
    // (6) TODO: liberar la memoria de pids y cargas
    

    printf("Todos los procesos terminaron.\n");
    return 0;
}
