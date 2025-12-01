# Taller Evaluables de Programación Concurrente y Paralela en C  
## Universidad Santiago de Cali
**Autor:** Diego Fernando Loaiza Buitrago  
**Curso:** Sistemas Operativos  
**Lenguaje:** C  
---

### Programación Concurrente con Hilos en C (POSIX pthreads) y programación paralela sin OpenMP

#### Objetivos
- Crear hilos con `pthread_create`.
- Sincronizar ejecución con `pthread_join`.
- Compartir datos usando paso de parámetros a hilos.
- Reconocer problemas de condición de carrera.
- Comprender la diferencia entre concurrencia con hilos y paralelismo con procesos.  
- Crear múltiples procesos usando `fork()`.  
- Compartir datos entre procesos mediante **memoria compartida POSIX** (`shm_open`, `mmap`).  
- Dividir una tarea matemática en partes y ejecutarla en paralelo.  

---

#### Ejercicio 1 — Completar el código (2.5 puntos)

**Actividad:** completar un programa donde **3 hilos imprimen secuencias de números**:

Hilo 1 → imprime del 1 al 5  
Hilo 2 → imprime del 6 al 10  
Hilo 3 → imprime del 11 al 15  

#### Código a completar:

```c
#include <stdio.h>
#include <pthread.h>

void* imprimir_rango(void* arg) {
    // TODO 1: recibir los datos enviados al hilo
    // ejemplo: Rango* r = (Rango*)arg;
    // for (int i = r->inicio; i <= r->fin; i++) printf("%d ", i);
    // pthread_exit(NULL);
}

typedef struct {
    int inicio;
    int fin;
} Rango;

int main() {

    pthread_t hilos[3];
    Rango rangos[3] = {
        {1, 5},
        // TODO 2: completar rangos
        // {6, 10},
        // {11, 15}
    };

    // TODO 3: crear los hilos usando pthread_create
    // TODO 4: sincronizar usando pthread_join

    return 0;
}

````

#### Preguntas teóricas (2.0 puntos)

¿Qué diferencia principal existe entre un proceso y un hilo?

¿Qué sucede cuando dos hilos comparten una variable global sin sincronización?

¿Qué función se usa para esperar la finalización de un hilo?

¿Qué técnica usaría para evitar condiciones de carrera?

### Programación Concurrente con Hilos en C (POSIX pthreads) y programación paralela sin OpenMP

#### Ejercicio 2: Programación Paralela en C **sin OpenMP**  (2.5 puntos)
#### Usando procesos (fork) + memoria compartida POSIX  

#### Objetivos
- Comprender la diferencia entre concurrencia con hilos y paralelismo con procesos.  
- Crear múltiples procesos usando `fork()`.  
- Compartir datos entre procesos mediante **memoria compartida POSIX** (`shm_open`, `mmap`).  
- Dividir una tarea matemática en partes y ejecutarla en paralelo.  

####  Descripción del ejercicio

Implementar un programa en C que calcule la **suma de los primeros 1,000,000 números**, paralelizando la tarea en **4 procesos hijo**, sin usar hilos ni OpenMP.

Cada proceso debe:
- Calcular una parte del rango.  
- Guardar su suma parcial en la memoria compartida.  
- El proceso padre debe reproducir la suma total.  

---

#### Código base (Completar)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define N 1000000
#define PROCESOS 4

int main() {

    // --- TODO 1: Crear memoria compartida ---
    // int fd = shm_open(...);
    // ftruncate(fd, PROCESOS * sizeof(long long));
    // long long* shm = mmap(...);

    int rango = N / PROCESOS;

    for (int p = 0; p < PROCESOS; p++) {

        pid_t pid = fork();

        if (pid == 0) {
            // --- PROCESO HIJO ---

            int inicio = p * rango + 1;
            int fin = (p == PROCESOS - 1) ? N : inicio + rango - 1;

            long long suma_parcial = 0;

            // --- TODO 2: Calcular la suma del rango asignado ---
            // for (...) suma_parcial += i;

            // --- TODO 3: Guardar en memoria compartida ---
            // shm[p] = suma_parcial;

            exit(0);
        }
    }

    // --- TODO 4: Esperar a todos los procesos hijo ---
    // for (...) wait(NULL);

    long long suma_total = 0;

    // --- TODO 5: Acumular resultados desde memoria compartida ---
    // for (...) suma_total += shm[i];

    printf("Suma total = %lld\n", suma_total);

    // --- TODO 6: Liberar memoria compartida ---
    // munmap(...)
    // shm_unlink(...)

    return 0;
}

```
#### Preguntas teóricas

- ¿Por qué este ejercicio es paralelo y no solo concurrente?
- ¿Qué diferencia clave hay entre `fork()` y `pthread_create()`?
- ¿Por qué se necesita memoria compartida y no una variable global?
- ¿Qué función se usa para esperar a un proceso hijo?

### Referencias

- OpenMP Application Programming Interface v5.2 https://www.openmp.org/wp-content/uploads/OpenMP-API-Specification-5-2.pdf
- POSIX Threads Programming — LLNL https://computing.llnl.gov/tutorials/pthreads/
