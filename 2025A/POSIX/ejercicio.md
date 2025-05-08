# Ejercicio POSIX en ANSI C para Linux: Monitoreo de Archivos con fork, stat y signal

## Objetivo

Construir una pequeña aplicación en C que use llamadas POSIX para:

- Crear un proceso hijo (`fork()`).
- Monitorear si un archivo ha sido modificado usando `stat()`.
- Recibir señales (`signal()`), como `SIGUSR1`, para imprimir información relevante.
- Terminar el proceso hijo si el archivo es eliminado.

---

## Requisitos

- Linux o sistema POSIX.
- Compilador GCC.
- Terminal bash.
- Conocimientos básicos de C.

---

## Descripción del Comportamiento

1. El proceso padre crea un proceso hijo.
2. El proceso hijo monitorea periódicamente un archivo (cada 3 segundos) para detectar cambios en la última fecha de modificación (`mtime`).
3. Si el archivo cambia, el hijo imprime un mensaje.
4. Si recibe `SIGUSR1`, el hijo imprime la fecha de última modificación.
5. Si el archivo es eliminado, el hijo termina.
6. El proceso padre espera al hijo y puede enviarle señales.

---

## Código a completar

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

volatile sig_atomic_t print_time = 0;

// Manejador de señal SIGUSR1
void handle_sigusr1(int signum) {
    print_time = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <ruta_del_archivo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filepath = argv[1];
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Proceso hijo
        signal(SIGUSR1, handle_sigusr1);

        struct stat file_stat;
        time_t last_mtime = 0;

        while (1) {
            if (stat(filepath, &file_stat) == -1) {
                if (errno == ENOENT) {
                    printf("Archivo eliminado. Hijo termina.\n");
                    exit(EXIT_SUCCESS);
                } else {
                    perror("stat");
                    exit(EXIT_FAILURE);
                }
            }

            if (last_mtime != file_stat.st_mtime) {
                printf("Cambio detectado en el archivo.\n");
                last_mtime = file_stat.st_mtime;
            }

            if (print_time) {
                printf("Última modificación: %s", ctime(&last_mtime));
                print_time = 0;
            }

            sleep(3);
        }

    } else {
        // Proceso padre
        printf("Proceso hijo creado con PID: %d\n", pid);
        printf("Envíe señal SIGUSR1 con: kill -SIGUSR1 %d\n", pid);
        printf("Esperando a que el hijo termine...\n");
        wait(NULL);
        printf("Hijo terminó correctamente.\n");
    }

    return 0;
}

```

### Completar o extender este ejercicio:

1. Agregar manejo de otra señal (SIGINT) para que el hijo termine limpiamente.
2. Permitir al padre enviar señales desde el mismo programa (usa sleep() + kill()).
3. Mostrar la diferencia en segundos desde la última modificación.
4. Crear logs en un archivo .log con cada cambio detectado.

### Referencias:

https://www.baeldung.com/linux/posix

https://man7.org/linux/man-pages/

