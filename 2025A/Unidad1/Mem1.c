#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    size_t size = 100 * 1024 * 1024; // 100 MB
    printf("Asignando %lu MB de memoria...\n", size / (1024 * 1024));

    char *buffer = (char *)malloc(size);
    if (buffer == NULL) {
        perror("Error en malloc");
        return 1;
    }

    printf("Memoria asignada. Presiona Enter para continuar...\n");
    getchar(); // Permite revisar memoria con vmstat o top

    free(buffer);
    printf("Memoria liberada. Presiona Enter para salir...\n");
    getchar();

    return 0;
}
