#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *archivo;
    char texto[100];

    // 1. Crear y escribir en el archivo
    archivo = fopen("datos.txt", "w");
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return 1;
    }
    fprintf(archivo, "Este es un ejemplo de gestión de archivos en C.\n");
    fclose(archivo);

    // 2. Leer el archivo
    archivo = fopen("datos.txt", "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo para lectura.\n");
        return 1;
    }
    printf("Contenido del archivo:\n");
    while (fgets(texto, sizeof(texto), archivo) != NULL) {
        printf("%s", texto);
    }
    fclose(archivo);

    return 0;
}
