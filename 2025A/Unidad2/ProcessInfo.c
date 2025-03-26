#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>

int main() {
    pid_t pid = getpid();      // Obtener el PID del proceso actual
    pid_t ppid = getppid();    // Obtener el PID del proceso padre
    int priority = getpriority(PRIO_PROCESS, pid);  // Obtener prioridad

    printf("📌 Información del proceso:\n");
    printf("🔹 PID: %d\n", pid);
    printf("🔹 PPID: %d\n", ppid);
    printf("🔹 Prioridad: %d\n", priority);

    return 0;
}
