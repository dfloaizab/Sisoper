// Lab2_shm_template.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX_PEDIDOS 5
#define PEDIDO_LEN 64

// Estructura del pedido
typedef struct {
    int cliente_id;
    char pedido[PEDIDO_LEN];
    int estado; // 0: libre, 1: recibido, 2: preparado
} Pedido;

// Estructura completa de memoria compartida
typedef struct {
    Pedido cola[MAX_PEDIDOS];
    int head;  // índice de próximo pedido a atender
    int tail;  // índice donde el cliente escribe
} BufferCompartido;

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// Semáforos: 0 = mutex, 1 = espacio disponible, 2 = pedidos disponibles
int sem_id;

// Función de inicialización de semáforos
void init_semaforos() {
    sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    semctl(sem_id, 0, SETVAL, 1);         // mutex
    semctl(sem_id, 1, SETVAL, MAX_PEDIDOS); // espacios disponibles
    semctl(sem_id, 2, SETVAL, 0);         // pedidos disponibles
}

// Operación P (wait)
void sem_wait(int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    semop(sem_id, &op, 1);
}

// Operación V (signal)
void sem_signal(int sem_num) {
    struct sembuf op = {sem_num, +1, 0};
    semop(sem_id, &op, 1);
}

// Cliente: escribe pedido
void cliente(int id) {

    //Conectarse a la memoria compartida
    int shm_id = shmget(SHM_KEY, sizeof(BufferCompartido), 0666);
    BufferCompartido *buffer = (BufferCompartido *) shmat(shm_id, NULL, 0);

    char comida[PEDIDO_LEN];
    printf("Cliente %d - Ingrese su pedido: ", id);
    fgets(comida, PEDIDO_LEN, stdin);
    comida[strcspn(comida, "\n")] = 0;

    //Implementar uso de semáforos (signal y wait) 
    //Y de la cola para registrar el pedido

    //Desvincular memoria compatrtida:
    shmdt(buffer);
}

// Cocina: atiende pedido
void cocina() {
    int shm_id = shmget(SHM_KEY, sizeof(BufferCompartido), IPC_CREAT | 0666);
    BufferCompartido *buffer = (BufferCompartido *) shmat(shm_id, NULL, 0);

    init_semaforos(); // Solo una vez al iniciar

    //Espera activa de los pedidos que llegan del cliente:
    while (1) {
      //Atender pedido de acuerdo a las señales de los semáforos
    }

    shmdt(buffer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s [cliente ID | cocina]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "cocina") == 0) {
        cocina();
    } else {
        int id = atoi(argv[1]);
        cliente(id);
    }

    return 0;
}
