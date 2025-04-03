#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 1024  // Tamaño del segmento de memoria compartida

int main() {
    key_t key = ftok("shmfile", 65); // Genera una clave única
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT); // Crea el segmento de memoria compartida
    char *data = (char *)shmat(shmid, (void *)0, 0); // Adjunta el segmento
    
    if (fork() == 0) {
        // Proceso hijo - escritor
        printf("Escribiendo en la memoria compartida...\n");
        strcpy(data, "Hola desde el proceso hijo!");
        printf("Mensaje enviado: %s\n", data);
        shmdt(data); // Desadjunta la memoria
    } else {
        // Proceso padre - lector
        sleep(2); // Espera un momento para asegurar que el hijo escriba primero
        printf("Leyendo de la memoria compartida...\n");
        printf("Mensaje recibido: %s\n", data);
        shmdt(data); // Desadjunta la memoria
        shmctl(shmid, IPC_RMID, NULL); // Elimina el segmento de memoria compartida
    }
    return 0;
}
