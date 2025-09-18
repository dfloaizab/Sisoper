//programa ansi c - manejo de procesos

#include <stdio.h>
#include <unistd.h>   // fork(), getpid(), sleep() //llamados al sistema
#include <sys/wait.h> // wait()                    


int main() {
    printf("Proceso principal (padre): PID=%d\n", getpid());

    // Crear un nuevo proceso (hijo)
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error al crear el proceso");
        return 1;
    }

    if (pid == 0) {
        // Proceso hijo
        printf("Soy el proceso HIJO: PID=%d, PPID=%d\n", getpid(), getppid());
        printf("El hijo está ejecutando su 'imagen' del programa.\n");
        sleep(2); // Simula que el hijo trabaja un poco
        printf("El hijo termina su ejecución.\n");
    } else {
        // Proceso padre
        printf("Soy el proceso PADRE: PID=%d, mi hijo tiene PID=%d\n", getpid(), pid);
        printf("El padre espera a que el hijo termine...\n");
        wait(NULL);
        printf("El padre detecta que el hijo terminó.\n");
    }

    return 0;
}
