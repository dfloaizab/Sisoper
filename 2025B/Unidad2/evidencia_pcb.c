/* evidencia_pcb.c
 *
 * Muestra elementos que permiten "evidenciar" el PCB y el process image:
 * - /proc/self/stat y /proc/self/status (campos tipo PCB)
 * - /proc/self/maps (segmentos: text, data, heap, stack, libs)
 * - addresses: &global (data), sbrk(0) (heap end), &local (stack),
 *   program counter mediante backtrace/dladdr
 *
 * Compilar:
 *   gcc -Wall -O2 -rdynamic -o evidencia_pcb evidencia_pcb.c -ldl
 *
 * Ejecutar:
 *   ./evidencia_pcb
 *
 * (No necesita privilegios especiales cuando inspecciona su propio proceso)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <execinfo.h>    /* backtrace */
#include <dlfcn.h>       /* dladdr */

static int global_var = 0x123456;   /* variable global -> segmento .data */

void print_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("No fue posible abrir %s: %s\n", path, strerror(errno));
        return;
    }
    char buf[4096];
    printf("----- %s -----\n", path);
    while (fgets(buf, sizeof(buf), f)) {
        fputs(buf, stdout);
    }
    printf("----- fin %s -----\n\n", path);
    fclose(f);
}

void print_stat_fields() {
    /* Lectura de /proc/self/stat (una línea con muchos campos).
       Para interpretación rápida mostramos la línea completa y algunas extracciones básicas. */
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/stat");
    FILE *f = fopen(path, "r");
    if (!f) { printf("No se pudo abrir %s: %s\n", path, strerror(errno)); return; }
    /* Leer línea entera */
    char line[4096];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    fclose(f);
    printf("Contenido /proc/self/stat (campos según man proc):\n%s\n\n", line);

    /* Extraer algunos campos importantes:
       (1) pid (campo 1)
       (3) state (campo 3)
       (4) ppid (campo 4)
       (14) utime, (15) stime
       (18) priority (campo 18), (19) nice (campo 19)
       (24) starttime (campo 22 depending; indexing varies). 
       Nota: parsing robusto para todos los campos requiere analizar cuidadosamente paréntesis del comm.
    */
    /* Hacemos un parsing simple: pid, comm, state, ppid, rest... */
    char comm[256], state;
    int pid, ppid;
    /* Format: pid (comm) state ppid ... */
    /* Use sscanf with %d ( then ) %c %d to get pid, comm, state, ppid */
    /* CUIDADO: comm puede contener espacios, por eso leemos entre paréntesis */
    char *open = strchr(line, '(');
    char *close = strrchr(line, ')');
    if (open && close && open < close) {
        *close = '\0';
        sscanf(line, "%d", &pid);
        strncpy(comm, open+1, sizeof(comm)-1);
        comm[sizeof(comm)-1] = '\0';
        sscanf(close+1, " %c %d", &state, &ppid);
        printf("Parsed: pid=%d\ncomm=%s\nstate=%c\nppid=%d\n\n", pid, comm, state, ppid);
    } else {
        printf("No se pudo parsear /proc/self/stat correctamente (formato inesperado).\n\n");
    }
}

void print_addresses() {
    /* sbrk(0) => fin actual del heap (end of data segment / program break) */
    void *heap_end = sbrk(0);
    int local_stack_var = 0;
    void *addr_local = (void*)&local_stack_var;
    void *addr_global = (void*)&global_var;

    printf("Direcciones útiles (hex):\n");
    printf("  &global_var (segmento .data) : %p\n", addr_global);
    printf("  sbrk(0) (end of heap / brk)  : %p\n", heap_end);
    printf("  &local_stack_var (stack)     : %p\n", addr_local);

    /* Program counter: obtenemos la dirección de retorno con __builtin_return_address o backtrace */
#if defined(__GNUC__)
    void *ret_addr = __builtin_return_address(0);
    printf("  __builtin_return_address(0)  : %p\n", ret_addr);
#else
    printf("  __builtin_return_address no disponible en este compilador.\n");
#endif

    /* Usar backtrace para obtener el stack de direcciones */
    void *bt[32];
    int n = backtrace(bt, 32);
    char **sym = backtrace_symbols(bt, n);
    printf("\nBacktrace (n=%d):\n", n);
    for (int i = 0; i < n; ++i) {
        Dl_info info;
        if (dladdr(bt[i], &info) && info.dli_sname) {
            printf("  #%d %p %s + %td\n", i, bt[i], info.dli_sname,
                   (info.dli_saddr ? (char*)bt[i] - (char*)info.dli_saddr : 0));
        } else {
            printf("  #%d %p %s\n", i, bt[i], sym ? sym[i] : "");
        }
    }
    if (sym) free(sym);
    printf("\n");
}

int main(int argc, char **argv) {
    printf("PID: %d\n\n", getpid());

    /* 1) /proc/self/status (información legible del proceso - refleja campos importantes del PCB) */
    print_file("/proc/self/status");

    /* 2) /proc/self/stat (campos compactos) y parseo parcial */
    print_stat_fields();

    /* 3) /proc/self/cmdline y /proc/self/exe */
    print_file("/proc/self/cmdline");
    /* /proc/self/exe es un enlace simbólico al ejecutable */
    char exe_path[4096];
    ssize_t r = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
    if (r > 0) {
        exe_path[r] = '\0';
        printf("/proc/self/exe -> %s\n\n", exe_path);
    }

    /* 4) /proc/self/maps (mapa de memoria virtual con segmentos: text, data, heap, stack, libs) */
    print_file("/proc/self/maps");

    /* 5) Mostrar direcciones representativas (data, heap, stack, program counter) */
    print_addresses();

    /* 6) Lectura de /proc/self/statm (resumen del uso de memoria en páginas) */
    print_file("/proc/self/statm");

    /* 7) Mostrar aviso sobre limitaciones (PCB real en kernel) */
    printf("Nota: la estructura PCB real está en el kernel y no es accesible en espacio usuario.\n");
    printf("Lo que mostramos son representaciones y campos expuestos por /proc que reflejan\n");
    printf("el estado del proceso (muchos de esos campos están almacenados o derivados del PCB).\n");

    return 0;
}
