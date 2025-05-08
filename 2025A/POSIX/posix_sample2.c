#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd[2];
    pipe(fd);

    if (fork() == 0) {
        close(fd[0]); // cerrar lectura
        char msg[] = "Hola desde el hijo";
        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);
    } else {
        close(fd[1]); // cerrar escritura
        char buffer[100];
        read(fd[0], buffer, sizeof(buffer));
        printf("Padre recibió: %s\n", buffer);
    }

    return 0;
}
