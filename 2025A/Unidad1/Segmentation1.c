#include <stdio.h>
#include <stdlib.h>

int global_var = 10; // Segmento de datos

void function() {
    int stack_var = 20; // Segmento de pila
    printf("Variable en la pila: %d\n", stack_var);
}

int main() {
    int *heap_var = (int *)malloc(sizeof(int)); // Segmento de heap
    if (heap_var == NULL) {
        perror("Error en malloc");
        return 1;
    }

    *heap_var = 30;
    
    printf("Variable global: %d\n", global_var);
    printf("Variable en el heap: %d\n", *heap_var);
    
    function();

    free(heap_var);
    return 0;
}
