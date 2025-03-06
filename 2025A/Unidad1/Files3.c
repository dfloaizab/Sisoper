#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>   // Para open()
#include <unistd.h>  // Para close(), read(), write()
#include <string.h>  // Para strlen()
#include <sys/stat.h> // Para stat()

int main() {
    int fd;
    char texto[100];
    char mensaje[] = "Este es un ejemplo de gestión de archivos en C.\n";
    struct stat info;

    // 1️. Crear y escribir en el archivo
    fd = open("datos.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error al abrir el archivo para escritura");
        return 1;
    }
    write(fd, mensaje, strlen(mensaje)); // Escribe el mensaje en el archivo
    close(fd);

    // 2️. Leer el archivo
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

    // 3️. Obtener información del archivo (inode, permisos, tamaño)
    if (stat("datos.txt", &info) == -1) {
        perror("Error al obtener información del archivo");
        return 1;
    }

    printf("\nInformación del archivo 'datos.txt':\n");
    printf("Inode: %ld\n", (long)info.st_ino);
    printf("Tamaño: %ld bytes\n", (long)info.st_size);
    printf("Permisos: %o (Octal)\n", info.st_mode & 0777);
    printf("Propietario: UID %d, GID %d\n", info.st_uid, info.st_gid);

    return 0;
}
