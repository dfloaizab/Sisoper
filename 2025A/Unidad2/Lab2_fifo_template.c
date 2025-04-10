// Lab2_fifo_template.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PEDIDOS "fifo_pedidos"
#define FIFO_RESPUESTAS "fifo_respuestas"
#define MAX_PEDIDO 128

typedef struct {
    int cliente_id;
    char mensaje[MAX_PEDIDO];
} Pedido;

// Implementar: función que inicializa los FIFOs si no existen

// Implementar: función para el cliente
void cliente(int id) {
    Pedido p;
    p.cliente_id = id;

    // 1. Leer pedido desde teclado
    // 2. Escribir en FIFO_PEDIDOS
    // 3. Leer confirmación desde FIFO_RESPUESTAS
    // 4. Mostrar mensaje recibido
}

// Implementar: función para la cocina
void cocina() {
    Pedido p;

    // 1. Crear los FIFOs si es necesario
    // 2. Bucle infinito para recibir pedidos
    // 3. Leer pedido desde FIFO_PEDIDOS
    // 4. Simular preparación
    // 5. Enviar mensaje de confirmación a FIFO_RESPUESTAS
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s [cliente ID | cocina]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "cocina") == 0) {
        cocina();
    } else {
        cliente(atoi(argv[1]));
    }

    return 0;
}
