#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

int main() {
    struct stat info;

    if (stat("archivo.txt", &info) == 0) {
        printf("Tamaño: %ld bytes\n", info.st_size);
        printf("Última modificación: %s", ctime(&info.st_mtime));
    } else {
        perror("Error al obtener información del archivo");
    }

    return 0;
}
