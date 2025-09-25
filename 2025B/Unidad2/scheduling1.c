#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void cpu_task(int n, const char *name) {
    for (int i = 0; i < n; i++) {
        if (i % (n/5+1) == 0) {
            printf("[%s] paso %d (pid=%d)\n", name, i, getpid());
            usleep(50000); // simula pausa breve
        }
    }
    printf("[%s] terminado (pid=%d)\n", name, getpid());
}

int main() {
    pid_t p1, p2, p3;

    if ((p1 = fork()) == 0) {
        cpu_task(5000000, "Corto");
        exit(0);
    }
    if ((p2 = fork()) == 0) {
        cpu_task(15000000, "Mediano");
        exit(0);
    }
    if ((p3 = fork()) == 0) {
        cpu_task(30000000, "Largo");
        exit(0);
    }

    // padre espera
    wait(NULL);
    wait(NULL);
    wait(NULL);

    printf("Todos los procesos terminaron.\n");
    return 0;
}
