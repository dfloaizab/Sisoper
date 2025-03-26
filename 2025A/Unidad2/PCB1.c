#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void show_process_info() {
    char path[50];
    char buffer[256];
    FILE *file;

    // Obtener el PID del proceso actual
    pid_t pid = getpid();
    printf("📌 Información del Proceso (PCB & Process Image)\n");
    printf("🔹 PID del proceso actual: %d\n", pid);

    // Construir la ruta al archivo /proc/[pid]/status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    // Abrir el archivo para leer información
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error al abrir /proc/[pid]/status");
        exit(EXIT_FAILURE);
    }

    // Leer y mostrar información del proceso
    printf("\n📂 Información extraída del PCB:\n");
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "Pid:", 4) == 0 ||
            strncmp(buffer, "PPid:", 5) == 0 ||
            strncmp(buffer, "State:", 6) == 0 ||
            strncmp(buffer, "Priority:", 9) == 0 ||
            strncmp(buffer, "Threads:", 8) == 0 ||
            strncmp(buffer, "VmSize:", 7) == 0 ||
            strncmp(buffer, "VmRSS:", 6) == 0) {
            printf("%s", buffer);
        }
    }

    fclose(file);
}

int main() {
    show_process_info();
    return 0;
}
