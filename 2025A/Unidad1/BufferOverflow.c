#include <stdio.h>

void vulnerable_function() {
    char buffer[10];  // Solo puede almacenar 9 caracteres + '\0'

    printf("Ingresa un texto: ");
    gets(buffer); 

    printf("Has ingresado: %s\n", buffer);
}

int main() {
    vulnerable_function();
    return 0;
}
