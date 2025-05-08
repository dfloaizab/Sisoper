#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void manejar_sigint(int sig) {
    printf("Capturada la señal SIGINT (Ctrl+C). Ignorando...\n");
}

int main() {
    signal(SIGINT, manejar_sigint); // Asocia SIGINT con función personalizada

    while (1) {
        printf("Trabajando... presiona Ctrl+C\n");
        sleep(2);
    }

    return 0;
}
