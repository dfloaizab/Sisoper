#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>   // Para open()
#include <unistd.h>  // Para close(), read(), write()
#include <string.h>  // Para strlen()

int main() {
    int fd;
    char texto[100];
    char mensaje[] = "Este es un ejemplo de gestión de archivos en C.\n";

    // 1. Crear y escribir en el archivo
    fd = open("datos.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo para escritura");
        return 1;
    }
    write(fd, mensaje, strlen(mensaje)); // Escribe el mensaje en el archivo
    close(fd);

    // 2. Leer el archivo
    fd = open("datos.txt", O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el archivo para lectura");
        return 1;
    }
    printf("Contenido del archivo:\n");
    int bytes_leidos = read(fd, texto, sizeof(texto) - 1);
    if (bytes_leidos > 0) {
        texto[bytes_leidos] = '\0'; // Agregar terminador de cadena
        printf("%s", texto);
    }
    close(fd);

    return 0;
}
