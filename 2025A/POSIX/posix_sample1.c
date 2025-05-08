#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", "-l", NULL); // hijo
    } else {
        wait(NULL); // padre espera al hijo
        printf("Proceso hijo terminado.\n");
    }

    return 0;
}
