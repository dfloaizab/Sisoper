/*
 * IPC usando archivos FIFO en Linux
 * Este programa muestra cómo un proceso puede comunicarse con otro
 * utilizando un FIFO (named pipe).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_PATH "./myfifo"
#define BUFFER_SIZE 100

// Función para el proceso escritor
void writer() {
    int fd;
    char message[] = "Hola desde el proceso escritor!";

    // Crear FIFO si no existe
    mkfifo(FIFO_PATH, 0666);

    // Abrir FIFO en modo escritura
    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir FIFO para escritura");
        exit(EXIT_FAILURE);
    }

    // Escribir en FIFO
    write(fd, message, strlen(message) + 1);
    close(fd);
}

// Función para el proceso lector
void reader() {
    int fd;
    char buffer[BUFFER_SIZE];

    // Abrir FIFO en modo lectura
    fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir FIFO para lectura");
        exit(EXIT_FAILURE);
    }

    // Leer desde FIFO
    read(fd, buffer, BUFFER_SIZE);
    printf("Proceso lector recibió: %s\n", buffer);
    close(fd);

    // Eliminar FIFO
    unlink(FIFO_PATH);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s [writer|reader]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "writer") == 0) {
        writer();
    } else if (strcmp(argv[1], "reader") == 0) {
        reader();
    } else {
        fprintf(stderr, "Opción inválida. Usa 'writer' o 'reader'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
