# Laboratorio de Sistemas Operativos 2025B
## Universidad Santiago de Cali
## Comunicación Cooperativa entre Procesos con Memoria Compartida y FIFO

## 1. Título
Comunicación Cooperativa entre Procesos: Sincronización y Colaboración usando Memoria Compartida y FIFO en C ANSI

## 2. Introducción
Este laboratorio tiene como propósito aplicar los principios fundamentales de la **comunicación entre procesos (IPC)** y el **manejo de concurrencia** en sistemas operativos tipo Unix/Linux. Se implementará un sistema cooperativo en el cual múltiples procesos colaboran para generar y procesar datos mediante **memoria compartida** y **archivos FIFO**.  

La práctica permitirá observar los efectos de la planificación del CPU, el cambio de contexto, y los problemas de sincronización que surgen en entornos concurrentes.  

## 3. Objetivos
1. Comprender la cooperación entre procesos mediante mecanismos IPC.  
2. Implementar comunicación mediante **memoria compartida** y **archivos FIFO**.  
3. Analizar los estados del ciclo de vida de un proceso y su interacción con el planificador.  
4. Implementar sincronización básica y manejo de concurrencia controlada.  
5. Extender un programa base para hacerlo más robusto mediante asignación dinámica y múltiples consumidores.  

## 4. Enunciado del problema
Se debe construir un sistema de **procesamiento cooperativo** entre un **Productor** y **dos Consumidores**, que trabajen de forma sincronizada para calcular resultados matemáticos a partir de pares de números enteros.  

El Productor genera pares `(a, b)` y los envía por un **FIFO**. Los Consumidores leen estos pares, almacenan los datos en una **memoria compartida**, realizan diferentes cálculos (por ejemplo, uno calcula `a × b` y otro `a + b`) y escriben los resultados en un arreglo dinámico.  

Debe garantizarse que:
- No se pierdan ni sobrescriban datos.
- Los procesos esperen correctamente antes de acceder a datos compartidos.
- Se liberen los recursos al finalizar.  

La solución debe usar **malloc** para la gestión dinámica de resultados y emplear un mecanismo simple de sincronización (por ejemplo, variables de estado).  

## 5. Código base en C ANSI (incompleto)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define FIFO_FILE "canal_fifo"
#define SHM_KEY 1234
#define BUFFER_SIZE 128

typedef struct {
    int a;
    int b;
    int resultado_mul;
    int resultado_sum;
    int listo;
} DatosCompartidos;

int main() {
    int shm_id;
    DatosCompartidos *mem;
    pid_t pid_prod, pid_cons1, pid_cons2;

    // Crear memoria compartida
    shm_id = shmget(SHM_KEY, sizeof(DatosCompartidos), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Error creando memoria compartida");
        exit(1);
    }

    mem = (DatosCompartidos *) shmat(shm_id, NULL, 0);
    if (mem == (DatosCompartidos *) -1) {
        perror("Error asociando memoria compartida");
        exit(1);
    }

    // Crear FIFO si no existe
    mkfifo(FIFO_FILE, 0666);

    pid_prod = fork();

    if (pid_prod == 0) {
        // Proceso Productor
        int fd = open(FIFO_FILE, O_WRONLY);
        if (fd < 0) {
            perror("Error abriendo FIFO para escritura");
            exit(1);
        }

        for (int i = 1; i <= 5; i++) {
            int a = i, b = i + 2;
            char buffer[BUFFER_SIZE];
            snprintf(buffer, BUFFER_SIZE, "%d %d", a, b);
            write(fd, buffer, strlen(buffer) + 1);
            printf("[Productor] Enviado: %s\n", buffer);
            sleep(1);
        }

        close(fd);
        exit(0);
    } else {
        // Crear dos consumidores
        pid_cons1 = fork();
        if (pid_cons1 == 0) {
            // Primer consumidor: calcula multiplicación
            // TODO: abrir FIFO para lectura
            // TODO: leer datos, copiar en memoria compartida y calcular a*b
            // TODO: usar malloc para crear un arreglo de resultados
            // TODO: imprimir resultados parciales
            //men->a, mem->b, mem->listo
            exit(0);
        } else {
            pid_cons2 = fork();
            if (pid_cons2 == 0) {
                // Segundo consumidor: calcula suma
                // TODO: abrir FIFO para lectura
                // TODO: leer datos, copiar en memoria compartida y calcular a+b
                // TODO: usar malloc para almacenar resultados sumados
                // TODO: imprimir resultados parciales
                exit(0);
            }
        }
    }

    // TODO: el proceso padre debe esperar a que todos los hijos terminen
    // TODO: liberar memoria compartida y eliminar el FIFO

    return 0;
}
```

### Comentarios sobre las partes incompletas
- Implementar el bloque de lectura en los consumidores (`open`, `read`, `sscanf`).  
- Implementar la creación de arreglos dinámicos (`malloc`) para almacenar resultados.  
- Incluir sincronización básica (por ejemplo, una variable `listo` o retardos controlados con `usleep`).  
- Añadir el manejo adecuado de `wait()` en el proceso padre.  
- Asegurar la liberación de la memoria compartida y el FIFO al finalizar.  

## 6. Preguntas de análisis
1. Describa los estados del ciclo de vida de cada proceso durante la ejecución del programa.  
2. ¿Qué mecanismo de IPC es más eficiente para el intercambio de datos en este caso y por qué?  
3. ¿Qué problemas de sincronización pueden presentarse y cómo pueden evitarse?  
4. ¿Cómo afectaría el algoritmo de planificación (Round Robin, CFS, etc.) al rendimiento del programa?  
5. ¿Qué impacto tendría cambiar el nivel de “niceness” del proceso Productor respecto a los Consumidores?  
6. ¿Qué sucede si un proceso intenta leer del FIFO cuando está vacío?  
7. Analice el comportamiento del cambio de contexto en este sistema.  
8. ¿Por qué es importante liberar la memoria compartida antes de finalizar?  
9. ¿Qué diferencias existen entre compartir datos con `shm` y hacerlo mediante archivos temporales?  

## 7. Recomendaciones
- Compilar con `gcc -o lab_ipc lab_ipc.c` y ejecutar en terminal Linux.  
- Verificar el segmento de memoria compartida con `ipcs -m`.  
- Eliminar manualmente recursos residuales con `ipcrm -m <id>` y `unlink("canal_fifo")`.  
- Monitorear procesos y prioridades con `top`, `htop`, o `ps -l`.  

## 8. Referencias
- Stallings, W. (2018). *Operating Systems: Internals and Design Principles* (9th ed.). Pearson Education. Capítulos 3 y 5.  
- Tanenbaum, A. S., & Bos, H. (2015). *Modern Operating Systems* (4th ed.). Pearson Education. Capítulo 4.  
- Kerrisk, M. (2010). *The Linux Programming Interface*. No Starch Press. Capítulo 49.  
- Love, R. (2010). *Linux Kernel Development* (3rd ed.). Addison-Wesley. Capítulo 6.  
