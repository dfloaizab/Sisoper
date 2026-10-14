# Sistemas Operativos 2026B - Universidad Santiago de Cali
## Guía de clase: Problemas de sincronización en IPC con FIFO y memoria compartida

**Laboratorio IPC**
**Comunicación cooperativa entre procesos (Productor y dos Consumidores) en C sobre Linux**

---

## Tabla de contenido

1. [Propósito de la sesión](#1-propósito-de-la-sesión)
2. [Por qué la sincronización es importante](#2-por-qué-la-sincronización-es-importante)
3. [Mapa del ejercicio](#3-mapa-del-ejercicio)
4. [Problemas de sincronización presentes en el código base](#4-problemas-de-sincronización-presentes-en-el-código-base)
5. [Estrategias de mitigación](#5-estrategias-de-mitigación)
6. [Demostración en vivo: condición de carrera](#6-demostración-en-vivo-condición-de-carrera)
7. [Comandos para verificar el comportamiento](#7-comandos-para-verificar-el-comportamiento)
8. [Experimentos guiados](#8-experimentos-guiados)
9. [Conexión con las preguntas de análisis del laboratorio](#9-conexión-con-las-preguntas-de-análisis-del-laboratorio)
10. [Actividad de extensión](#10-actividad-de-extensión)
11. [Referencias](#11-referencias)

---

## 1. Propósito de la sesión

Al finalizar la sesión, el estudiante estará en capacidad de:

1. Identificar **secciones críticas** y **condiciones de carrera** en un programa con varios procesos que comparten datos.
2. Distinguir qué garantiza y qué **no** garantiza un FIFO frente a la memoria compartida.
3. Aplicar mecanismos de sincronización (semáforos, protocolo de turnos, partición de datos) para evitar pérdida o sobrescritura de información.
4. Usar herramientas de Linux (`ipcs`, `ps`, `top`, `strace`, `/proc`) para **evidenciar** el comportamiento de los procesos y los recursos IPC.
5. Liberar correctamente los recursos del sistema (segmentos `shm`, FIFO, procesos zombis).

<!-- **Ruta sugerida de la clase:** (1) motivación y concepto → (2) lectura crítica del código base → (3) demostración de la condición de carrera → (4) mitigación con semáforos → (5) verificación con comandos → (6) preguntas de análisis. -->

---

## 2. Por qué la sincronización es importante

Cuando dos o más procesos acceden a un mismo recurso y al menos uno lo modifica, el resultado **depende del orden exacto en que el planificador intercala sus instrucciones**. A esto se le denomina **condición de carrera** (*race condition*).

| Concepto | Definición breve |
|---|---|
| **Sección crítica** | Fragmento de código que accede a un recurso compartido y que no debe ser ejecutado por dos procesos a la vez. |
| **Exclusión mutua** | Garantía de que, como máximo, un proceso está dentro de la sección crítica. |
| **Condición de carrera** | Resultado incorrecto producido por la intercalación no controlada de accesos. |
| **Espera activa** (*busy waiting*) | El proceso consulta una condición en un ciclo consumiendo CPU. |
| **Bloqueo / interbloqueo** (*deadlock*) | Procesos que esperan indefinidamente un evento que nunca ocurrirá. |
| **Inanición** (*starvation*) | Un proceso nunca obtiene el recurso porque otros lo adquieren siempre primero. |

**Razones para estudiarla:**

- **Los errores son no deterministas.** El programa puede funcionar 100 veces y fallar la 101. Depurar este tipo de fallos es costoso porque, al observarlos (por ejemplo con `printf`), el tiempo de ejecución cambia y el error puede desaparecer.
- **Consecuencias reales:** saldos bancarios incorrectos, inventarios duplicados, archivos corruptos, bases de datos inconsistentes.
- **Es transversal al diseño de SO:** el propio kernel debe sincronizar sus estructuras internas (tabla de procesos, colas de planificación, buffers de E/S).
- **La ausencia de fallos en las pruebas no demuestra corrección.** Un programa con una carrera puede pasar todas las pruebas en una máquina de un solo núcleo y fallar en un servidor multinúcleo.

---

## 3. Mapa del ejercicio

```
                    ┌──────────────┐
                    │  PRODUCTOR   │   genera pares (a, b), 1 por segundo
                    └──────┬───────┘
                           │ write()
                           ▼
                    ┌──────────────┐
                    │ FIFO         │   canal_fifo  (flujo de bytes, sin
                    │ canal_fifo   │   límites de mensaje)
                    └──────┬───────┘
                 read()    │    read()
              ┌────────────┴────────────┐
              ▼                         ▼
      ┌───────────────┐         ┌───────────────┐
      │ CONSUMIDOR 1  │         │ CONSUMIDOR 2  │
      │   a × b       │         │   a + b       │
      └───────┬───────┘         └───────┬───────┘
              │ lee/escribe             │ lee/escribe
              ▼                         ▼
        ┌────────────────────────────────────┐
        │  MEMORIA COMPARTIDA (System V)     │
        │  a | b | resultado_mul | resultado_sum | listo │
        └────────────────────────────────────┘

   malloc(): arreglos de resultados  →  PRIVADOS de cada proceso (no son compartidos)
```

**Aspecto conceptual clave:** el arreglo creado con `malloc` en cada consumidor reside en el *heap* **privado** del proceso. Tras el `fork`, cada proceso tiene su propia copia (con *copy-on-write*). Solo el segmento `shmget/shmat` es realmente compartido.

---

## 4. Problemas de sincronización presentes en el código base

### 4.1 Resumen

| # | Problema | Dónde aparece | Síntoma observable |
|---|---|---|---|
| P1 | **Lectores en competencia sobre un mismo FIFO** | Ambos consumidores hacen `open(FIFO_FILE, O_RDONLY)` y `read` | Cada par `(a, b)` lo recibe **un solo** consumidor; `a×b` y `a+b` nunca corresponden al mismo par |
| P2 | **Estructura compartida de un solo espacio sin protección** | `DatosCompartidos` tiene un único `a`, `b` | Sobrescritura de datos: un consumidor escribe `a, b` antes de que el otro los lea |
| P3 | **`listo` como bandera sin atomicidad** | Campo `int listo` | Verificar y modificar la bandera son dos pasos separados; dos procesos pueden pasar la verificación a la vez |
| P4 | **Espera activa** | Ciclo `while (!mem->listo)` | Consumo de CPU cercano al 100 % mientras se espera |
| P5 | **Sin visibilidad de memoria** | Lectura repetida de `listo` sin `volatile` ni barreras | Con optimización (`-O2`) el compilador puede dejar el valor en un registro y el ciclo no termina nunca |
| P6 | **El FIFO es un flujo de bytes, no de mensajes** | `read(fd, buf, BUFFER_SIZE)` | Un `read` puede devolver **dos mensajes juntos** (`"1 3\0" "2 4\0"`); `sscanf` solo procesa el primero |
| P7 | **`sleep()` usado como sincronización** | `sleep(1)` en el productor; `usleep` sugerido en las recomendaciones | Funciona solo si se cumplen supuestos de tiempo; en una máquina cargada se rompe |
| P8 | **Sin señal de fin de datos para el consumidor que lee de `shm`** | No existe un campo `fin` | El consumidor se queda esperando para siempre (bloqueo) después del último par |
| P9 | **`wait()` ausente o incompleto en el padre** | TODO final | Procesos **zombis** (`Z`) y limpieza prematura de recursos |
| P10 | **Recursos huérfanos** | `shmget` con clave fija `1234`; `mkfifo` sin `unlink` | Segmentos `shm` y FIFO que sobreviven al programa; choque de claves entre estudiantes en una misma máquina |
| P11 | **Pérdida de memoria** | `malloc` sin `free` en los consumidores | Fuga de memoria (detectable con `valgrind`) |

### 4.2 Explicación de los problemas principales

#### P1. Lectores en competencia sobre el mismo FIFO

Un FIFO **entrega cada byte a un único lector**. Si dos procesos leen del mismo FIFO, el kernel reparte los mensajes entre ellos de forma no determinista (comportamiento de *cola de trabajo*, no de *difusión*).

Consecuencia: si el Consumidor 1 recibe los pares 1, 3 y 5 y el Consumidor 2 recibe los pares 2 y 4, los resultados parciales no son comparables.

**Decisión de diseño que conviene fijar al inicio de la clase** (el enunciado admite varias lecturas):

| Opción | Descripción | Observación |
|---|---|---|
| **A (recomendada)** | Solo el Consumidor 1 lee el FIFO y **publica** cada par en la memoria compartida; el Consumidor 2 lo toma desde allí | Requiere sincronización real (productor-consumidor con buffer de un elemento). Es la más rica didácticamente |
| **B** | Dos FIFO, uno por consumidor; el productor escribe cada par en ambos | Elimina la competencia; la sincronización queda en el FIFO |
| **C** | Ambos compiten por el mismo FIFO y cada uno calcula ambas operaciones | Es reparto de trabajo; el «consumidor de producto» y el «consumidor de suma» pierden sentido |

El resto de esta guía asume la **Opción A**.

#### P2, P3, P4 y P5. Memoria compartida sin protección

Consideremos el protocolo ingenuo con la bandera `listo`:

```c
/* Consumidor 1 */                      /* Consumidor 2 */
mem->a = a;                             while (!mem->listo) ;     /* espera activa */
mem->b = b;                             suma = mem->a + mem->b;
mem->listo = 1;                         mem->listo = 0;
```

Fallos posibles:

1. **Sobrescritura:** el Consumidor 1 lee el siguiente par del FIFO y escribe `mem->a` antes de que el Consumidor 2 haya terminado de usar el anterior. El enunciado exige expresamente que *no se pierdan ni sobrescriban datos*.
2. **Lectura a medias:** el Consumidor 2 podría leer `a` del par nuevo y `b` del par anterior si el Consumidor 1 es interrumpido entre las dos asignaciones.
3. **Verificar-y-actuar no atómico:** `if (!mem->listo) { ... mem->listo = 1; }` son varias instrucciones de máquina; el planificador puede interrumpir en medio.
4. **Espera activa:** `while (!mem->listo);` mantiene la CPU ocupada. Se observa con `top`.
5. **Visibilidad:** sin `volatile` (o una primitiva de sincronización que actúe como barrera), el compilador puede optimizar la lectura repetida de `listo`.

> **Mensaje para el estudiante:** una bandera entera es útil como *indicador de estado*, pero **no sustituye** a un mecanismo de exclusión mutua.

#### P6. El FIFO no preserva los límites de los mensajes

El productor hace `write(fd, "1 3\0", 4)`, `write(fd, "2 4\0", 4)`, etc. Para el kernel solo existe una secuencia de bytes. Si el consumidor se retrasa, un único `read` de 128 bytes puede devolver varios mensajes concatenados.

Mitigaciones, de menor a mayor robustez:

- Recorrer el búfer separando por el terminador `\0` (ver sección 5.2).
- Usar un delimitador explícito (`\n`) y leer línea por línea con `fdopen` + `fgets`.
- Usar mensajes de **tamaño fijo** (`struct { int a, b; }`) y leer exactamente `sizeof` bytes.

#### P8. Fin de datos

Cuando el Consumidor 1 detecta el fin del FIFO (`read` devuelve `0` porque el productor cerró su extremo), el Consumidor 2 no tiene forma de saberlo. Se debe añadir un campo `fin` en `DatosCompartidos` y despertarlo con la misma primitiva que usa para recibir datos.

#### P9 y P10. Ciclo de vida y limpieza

- Un hijo que termina y no es recogido con `wait()` queda en estado **zombi** (`Z`).
- Un segmento `shm` persiste en el kernel hasta que alguien ejecute `shmctl(id, IPC_RMID, NULL)` o `ipcrm`. Si el programa termina con error, el segmento queda ahí.
- Un FIFO es un archivo especial en el sistema de archivos; permanece hasta `unlink`.
- La clave fija `1234` puede colisionar con la de otro usuario y, si ya existe un segmento de menor tamaño con esa clave, `shmget` falla con `EINVAL`.

---

## 5. Estrategias de mitigación

### 5.1 Principio general: reducir lo que se comparte

Antes de agregar un mecanismo de sincronización, conviene preguntarse **si realmente es necesario compartir esa escritura**.

| Estrategia | Idea | Ejemplo en el laboratorio |
|---|---|---|
| **Particionar datos** | Cada proceso escribe en su propia zona | `resultado_mul` solo lo escribe el Consumidor 1 y `resultado_sum` solo el Consumidor 2: no hay conflicto |
| **Usar el canal como sincronizador** | `read` sobre un FIFO bloquea hasta que haya datos | Un FIFO por consumidor (Opción B) |
| **Exclusión mutua** | Un semáforo o *mutex* protege la sección crítica | Contador compartido de la demostración |
| **Señalización por eventos** | Semáforos que indican «lleno» y «vacío» | Buffer de un elemento (Opción A) |

### 5.2 Lectura robusta del FIFO (P6)

```c
char buf[BUFFER_SIZE];
ssize_t r;

while ((r = read(fd, buf, BUFFER_SIZE)) > 0) {
    /* Recorre todos los mensajes que hayan llegado en este read */
    for (char *p = buf; p < buf + r; p += strlen(p) + 1) {
        int a, b;
        if (sscanf(p, "%d %d", &a, &b) == 2) {
            /* procesar el par (a, b) */
        }
    }
}
/* r == 0  →  el productor cerró el FIFO (fin de datos) */
```

### 5.3 Buffer de un elemento con semáforos (P2–P5, P7, P8)

Es el esquema clásico **productor-consumidor con buffer acotado (N = 1)**. Se usan dos semáforos POSIX **sin nombre** ubicados dentro de la memoria compartida (`pshared = 1`):

- `vacio` (valor inicial 1): indica que el espacio está libre para escribir.
- `lleno` (valor inicial 0): indica que hay un par disponible para leer.

**Estructura modificada:**

```c
#include <semaphore.h>

typedef struct {
    int a;
    int b;
    int resultado_mul;
    int resultado_sum;
    int listo;
    int fin;          /* 1 = no habrá más pares (P8) */
    sem_t vacio;      /* 1 = el buffer está libre */
    sem_t lleno;      /* 1 = hay un par disponible */
} DatosCompartidos;
```

**Inicialización (en el padre, después de `shmat` y antes de cualquier `fork`):**

```c
memset(mem, 0, sizeof(*mem));
sem_init(&mem->vacio, 1, 1);   /* 2.º parámetro = 1 → compartido entre procesos */
sem_init(&mem->lleno, 1, 0);
```

**Consumidor 1 (único lector del FIFO; publica y calcula `a×b`):**

```c
/* por cada par (a, b) leído del FIFO */
sem_wait(&mem->vacio);          /* espera a que el Consumidor 2 haya tomado el par anterior */
mem->a = a;
mem->b = b;
sem_post(&mem->lleno);          /* avisa: hay un par nuevo */
res_mul[n++] = a * b;           /* arreglo propio, creado con malloc */

/* al detectar fin del FIFO (read devolvió 0) */
sem_wait(&mem->vacio);
mem->fin = 1;
sem_post(&mem->lleno);          /* despierta al Consumidor 2 para que termine */
```

**Consumidor 2 (lee desde la memoria compartida; calcula `a+b`):**

```c
for (;;) {
    sem_wait(&mem->lleno);      /* se bloquea (sin gastar CPU) hasta que haya dato */
    if (mem->fin) break;
    int a = mem->a, b = mem->b;
    sem_post(&mem->vacio);      /* libera el espacio lo antes posible */
    res_sum[n++] = a + b;       /* arreglo propio, creado con malloc */
}
```

**Por qué funciona:**

- `sem_wait` es **atómico** y **bloqueante**: el proceso pasa a estado `S` (dormido) sin consumir CPU. Esto resuelve P3 y P4.
- Las primitivas de semáforo incluyen las barreras de memoria necesarias, lo que resuelve P5.
- `vacio`/`lleno` imponen el orden *escribir → leer → escribir* y evitan la sobrescritura (P2).
- El campo `fin` evita el bloqueo del Consumidor 2 (P8).
- No depende de `sleep` para el orden correcto (P7).

**Compilación:** `gcc -Wall -pthread -o lab_ipc lab_ipc.c`

> Alternativa con la misma API del enunciado: semáforos System V (`semget`, `semop`, `semctl`). Los semáforos POSIX resultan más cortos y legibles para una primera práctica.

### 5.4 Cierre ordenado y limpieza (P9, P10, P11)

**En el padre, al final:**

```c
for (int i = 0; i < 3; i++) wait(NULL);   /* productor + 2 consumidores */

sem_destroy(&mem->vacio);
sem_destroy(&mem->lleno);
shmdt(mem);                                /* desasocia */
shmctl(shm_id, IPC_RMID, NULL);            /* marca el segmento para eliminación */
unlink(FIFO_FILE);                         /* elimina el archivo FIFO */
```

**En cada hijo:** `shmdt(mem)`, `close(fd)` y `free()` de todo lo reservado con `malloc` antes de `exit`.

**Buenas prácticas adicionales:**

- Generar la clave con `ftok(".", 'A')` o usar `IPC_PRIVATE` cuando el segmento solo se comparte entre padre e hijos, para evitar colisiones.
- Instalar un manejador de `SIGINT` en el padre que ejecute la limpieza si el usuario interrumpe con `Ctrl+C`.
- Verificar siempre el valor de retorno de `shmget`, `shmat`, `open`, `mkfifo`, `fork`.

---

## 6. Demostración en vivo: condición de carrera

Dos procesos incrementan **100 000 veces cada uno** (`N`) un contador compartido. Valor esperado: `2 × N` = 200 000. Sin protección, el resultado es incorrecto; con semáforo, exacto.

Guardar como `demo_carrera.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>

#define N 100000

typedef struct {
    volatile long contador;
    sem_t mutex;
} Compartido;

int main(int argc, char **argv) {
    int usar_sem = (argc > 1 && argv[1][0] == 'c');   /* "c" = con semáforo */

    int id = shmget(IPC_PRIVATE, sizeof(Compartido), IPC_CREAT | 0600);
    Compartido *c = (Compartido *) shmat(id, NULL, 0);
    c->contador = 0;
    sem_init(&c->mutex, 1, 1);

    for (int p = 0; p < 2; p++) {
        if (fork() == 0) {
            for (long i = 0; i < N; i++) {
                if (usar_sem) sem_wait(&c->mutex);      /* ENTRADA a la sección crítica */

                long tmp = c->contador;                 /* leer   */
                sched_yield();                          /* amplía la ventana de la carrera */
                c->contador = tmp + 1;                  /* escribir */

                if (usar_sem) sem_post(&c->mutex);      /* SALIDA de la sección crítica */
            }
            shmdt(c);
            exit(0);
        }
    }
    wait(NULL);
    wait(NULL);

    printf("%s | esperado: %d | obtenido: %ld\n",
           usar_sem ? "CON semaforo" : "SIN semaforo", 2 * N, c->contador);

    sem_destroy(&c->mutex);
    shmdt(c);
    shmctl(id, IPC_RMID, NULL);
    return 0;
}
```

(El valor de `N` en este fragmento es 100 000 porque `sched_yield()` hace cada iteración más lenta; el valor esperado impreso es `2 × N`.)

```bash
gcc -Wall -pthread -o demo_carrera demo_carrera.c
./demo_carrera sin     # sin protección
./demo_carrera con     # con semáforo
```

**Resultado esperado:**

```
SIN semaforo | esperado: 200000 | obtenido: <valor entre 100000 y 199999>
CON semaforo | esperado: 200000 | obtenido: 200000
```

**Puntos de discusión:**

- Con `sched_yield()` entre la lectura y la escritura se fuerza el cambio de contexto en medio de la sección crítica. En una máquina de **un solo núcleo** el resultado sin semáforo puede ser *exactamente* `N` (los dos procesos leen el mismo valor y ambos escriben `valor + 1`); en una máquina **multinúcleo** el valor varía de ejecución en ejecución.
- Si se elimina `sched_yield()` y se deja `c->contador++`, en un solo núcleo la carrera puede **no manifestarse** en miles de ejecuciones, y en multinúcleo sí. Esto ilustra que **la ausencia de síntomas no demuestra corrección**.
- Consultar el número de núcleos: `nproc`.

---

## 7. Comandos para verificar el comportamiento

Ejecute el programa en una terminal y use **otra terminal** para los comandos. Para observar con calma, aumente los `sleep` del productor o agregue una pausa final en los consumidores.

### 7.1 Procesos y jerarquía

| Comando | Qué muestra | Cómo interpretarlo |
|---|---|---|
| `ps -ef \| grep lab_ipc` | Todos los procesos del programa | La columna **PPID** de los tres hijos debe coincidir con el **PID** del padre |
| `ps -l` | Formato largo del usuario actual | **S** = estado; **PRI/NI** = prioridad; **WCHAN** = función del kernel en la que duerme |
| `ps -o pid,ppid,stat,wchan:20,pcpu,cmd -C lab_ipc` | Estado, punto de espera y consumo de CPU | Un consumidor bloqueado en `read` mostrará `S` y un `wchan` del tipo `pipe_read`; uno en `sem_wait`, un `wchan` del tipo `futex_...`; uno en `sleep`, `hrtimer_nanosleep` o similar (los nombres varían según la versión del kernel) |
| `pstree -p <PID_padre>` | Árbol de procesos | Verifica la relación padre-hijos |
| `ps -eo pid,ppid,stat,cmd \| awk '$3 ~ /Z/'` | Procesos zombis | Si aparece uno del programa, falta un `wait()` |
| `watch -n 0.5 'ps -o pid,ppid,stat,wchan:20,pcpu,cmd -C lab_ipc'` | Actualización continua | Permite ver cómo los procesos cambian de estado y desaparecen |

**Estados más comunes (`STAT`):**

| Código | Significado | Dónde aparece en este laboratorio |
|---|---|---|
| `R` | En ejecución o lista | Espera activa; cálculo |
| `S` | Dormido (interrumpible) | `sleep`, `read` sobre FIFO vacío, `sem_wait` |
| `D` | Dormido ininterrumpible (E/S) | Poco frecuente aquí |
| `T` | Detenido (`SIGSTOP`/`Ctrl+Z`) | Experimento 8.5 |
| `Z` | Zombi | Padre sin `wait()` |
| `+` | En primer plano | Sufijo del grupo de procesos en primer plano |

### 7.2 Uso de CPU y cambios de contexto

| Comando | Qué muestra | Cómo interpretarlo |
|---|---|---|
| `top -p $(pgrep -d',' lab_ipc)` | Solo los procesos del programa | Con espera activa, `%CPU` ≈ 100 % en el proceso que espera; con semáforos o `read` bloqueante, ≈ 0 % |
| `grep ctxt /proc/<PID>/status` | `voluntary_ctxt_switches` y `nonvoluntary_ctxt_switches` | **Voluntarios:** el proceso cede la CPU porque se bloquea (E/S, `sem_wait`, `sleep`). **No voluntarios:** el planificador lo expulsa (típico de la espera activa) |
| `pidstat -w -p <PID> 1` (paquete `sysstat`) | Cambios de contexto por segundo (`cswch/s`, `nvcswch/s`) | Misma lectura que el punto anterior, en tiempo real |
| `htop` (`F5` vista de árbol; `F4` filtrar por `lab_ipc`) | Vista interactiva | Útil para proyectar en clase |

En `top`: `H` alterna hilos, `1` muestra cada núcleo, `q` sale.

### 7.3 Memoria compartida (System V)

| Comando | Qué muestra | Cómo interpretarlo |
|---|---|---|
| `ipcs -m` | Segmentos de memoria compartida | **key** (en hexadecimal: `1234` decimal = `0x000004d2`), **shmid**, **owner**, **perms**, **bytes**, **nattch**, **status** |
| `ipcs -m -p` | PID creador (`cpid`) y PID de la última operación (`lpid`) | Permite saber quién creó y quién tocó por última vez el segmento |
| `ipcs -m -t` | Hora de último `shmat`, `shmdt` y cambio | Útil para verificar que se hizo el `shmdt` |
| `ipcs -m -i <shmid>` | Detalle de un segmento | Alternativa a la combinación anterior |
| `ipcs -s` | Semáforos System V | Solo si se usa `semget` (los semáforos POSIX sin nombre dentro de `shm` **no** aparecen aquí) |
| `ipcs -l` | Límites del sistema para IPC | Tamaño máximo de segmento, número máximo de segmentos |

**Cómo leer `nattch`:** es el número de procesos asociados al segmento. Como `shmat` se hace antes de los `fork`, los hijos heredan la asociación: se espera ver **4** al inicio (padre + 3 hijos), bajando conforme cada hijo hace `shmdt` o termina. Si el padre ejecuta `shmctl(IPC_RMID)` con `nattch > 0`, la columna `status` mostrará `dest` y el segmento se destruirá en el último `shmdt`.

**Limpieza manual:**

```bash
ipcrm -m <shmid>        # por identificador
ipcrm -M 0x000004d2     # por clave
unlink canal_fifo       # o: rm canal_fifo
```

### 7.4 FIFO

| Comando | Qué muestra | Cómo interpretarlo |
|---|---|---|
| `ls -l canal_fifo` | Archivo del FIFO | El primer carácter del modo es **`p`** (*pipe*): `prw-r--r--` |
| `stat canal_fifo` | Tipo de archivo | `fifo` |
| `lsof canal_fifo` o `fuser -v canal_fifo` | Procesos que lo tienen abierto, y en qué modo | Columna `FD`: `r` lectura, `w` escritura. Debe verse un escritor (productor) y los lectores |
| `ls -l /proc/<PID>/fd` | Descriptores de archivo del proceso | El enlace del descriptor del FIFO apunta a la ruta de `canal_fifo` |
| `echo "7 8" > canal_fifo` | Inyecta un mensaje manualmente | Se bloquea hasta que haya un lector; sirve para ver cómo un lector «roba» mensajes |
| `cat canal_fifo` | Un lector intruso | Compite con los consumidores y se queda con parte de los mensajes (demuestra P1) |

### 7.5 Llamadas al sistema con `strace`

```bash
strace -f -tt -e trace=mknodat,openat,read,write,close,shmget,shmat,shmdt,shmctl,wait4,unlink ./lab_ipc
```

- `-f`: sigue a los procesos hijos.
- `-tt`: marca de tiempo con microsegundos (permite ver quién actuó primero).
- En sistemas más antiguos, `open` aparece en lugar de `openat`.
- `mkfifo` se implementa en glibc mediante `mknodat`/`mknod`; `fork` aparece como `clone` o `clone3`.
- Para separar la salida por proceso: `strace -ff -o traza ./lab_ipc` (genera `traza.<PID>`).

**Qué buscar:**

- Dos `read` sobre el mismo descriptor del FIFO en procesos distintos que reciben mensajes distintos (P1).
- `read(..., "1 3\0" "2 4\0", ...) = 8`: un solo `read` devolvió dos mensajes (P6).
- `read(...) = 0`: fin del FIFO.
- `wait4(...)` repetido tres veces en el padre.

### 7.6 Memoria dinámica

```bash
valgrind --leak-check=full --trace-children=yes ./lab_ipc
```

Informa los bloques de `malloc` no liberados en cada proceso hijo (P11).

> **Aviso:** `valgrind --tool=helgrind` y `-fsanitize=thread` están pensados para **hilos** dentro de un proceso. No están diseñados para detectar carreras entre **procesos** que comparten memoria mediante `shmget`.

---

## 8. Experimentos guiados

Cada experimento sigue el formato **hipótesis → acción → observación → conclusión**.

### 8.1 Competencia por el FIFO (P1)

1. Ejecute el programa con la Opción C (ambos consumidores leyendo del FIFO).
2. Observe qué consumidor imprime cada par.
3. En otra terminal, ejecute `cat canal_fifo` mientras el productor envía.
4. **Observación esperada:** los pares se reparten de forma no determinista y el `cat` «roba» algunos.
5. **Conclusión:** un FIFO entrega cada byte a un único lector.

### 8.2 Sobrescritura sin sincronización (P2)

1. Use el protocolo de la bandera `listo` sin semáforos.
2. Introduzca un retardo pequeño (`usleep(100000)`) en el Consumidor 2 antes de leer `mem->a` y `mem->b`.
3. Reduzca el `sleep` del productor a `usleep(10000)` y aumente el número de pares.
4. **Observación esperada:** el Consumidor 2 procesa pares repetidos y se pierden otros.
5. Repita con el esquema de semáforos (sección 5.3): ya no hay pérdida ni repetición.

### 8.3 Espera activa frente a espera bloqueante (P4)

1. Implemente primero el consumidor con `while (!mem->listo);` y ejecute `top -p $(pgrep -d',' lab_ipc)`.
2. Anote `%CPU` y los cambios de contexto (`grep ctxt /proc/<PID>/status`).
3. Cambie a `sem_wait` y repita.
4. **Observación esperada:** ≈ 100 % de CPU y muchos cambios no voluntarios con espera activa; ≈ 0 % y cambios voluntarios con `sem_wait`.

### 8.4 Visibilidad de memoria con optimización (P5)

1. Compile el consumidor con espera activa usando `gcc -O0` y luego `gcc -O2`.
2. Declare `listo` primero como `int` y luego como `volatile int`.
3. **Observación posible:** con `-O2` y sin `volatile`, el ciclo puede no terminar nunca porque el compilador mantiene `listo` en un registro. (El comportamiento exacto depende de la versión del compilador; sirve para discutir por qué el estándar no garantiza visibilidad sin primitivas de sincronización.)

### 8.5 Un proceso detenido dentro de la sección crítica

1. Con el esquema de semáforos, ejecute `kill -STOP <PID_consumidor2>` en medio de la ejecución.
2. Observe con `ps -o pid,stat,wchan:20,cmd -C lab_ipc` que el Consumidor 1 se bloquea en `sem_wait(&mem->vacio)`.
3. Reanude con `kill -CONT <PID_consumidor2>`.
4. **Conclusión:** la sincronización transmite el retraso de un proceso a los demás (acoplamiento). Si en lugar de `SIGSTOP` se usa `SIGKILL` en un proceso que **tenía** el semáforo, los demás quedan bloqueados para siempre (con semáforos System V existe la opción `SEM_UNDO` precisamente para este caso).

### 8.6 Recursos huérfanos (P10)

1. Ejecute el programa y, durante la ejecución, interrúmpalo con `Ctrl+C` (o `kill -9` al padre).
2. Verifique: `ipcs -m` y `ls -l canal_fifo`.
3. **Observación esperada:** el segmento y el FIFO siguen existiendo; si se vuelve a ejecutar con otro tamaño de estructura, `shmget` puede fallar con `EINVAL`.
4. Limpie con `ipcrm` y `unlink`.

### 8.7 Procesos zombis (P9)

1. Comente temporalmente los `wait()` del padre y agregue `sleep(30)` al final del `main`.
2. Ejecute `ps -eo pid,ppid,stat,cmd | awk '$3 ~ /Z/'`.
3. **Observación esperada:** hijos con estado `Z` y `<defunct>`.

---

## 9. Conexión con las preguntas de análisis del laboratorio

Orientación para el docente: qué concepto y qué evidencia relacionar con cada pregunta.

| Pregunta del laboratorio | Concepto central | Evidencia sugerida |
|---|---|---|
| **1.** Estados del ciclo de vida | Nuevo → listo → ejecución → bloqueado → terminado (y zombi) | `ps -o pid,stat,wchan:20,cmd` en diferentes momentos; `watch` |
| **2.** Mecanismo IPC más eficiente | Memoria compartida evita copias a través del kernel (tras el `shmat`, el acceso es directo); el FIFO copia datos usuario → kernel → usuario y además sincroniza gracias al bloqueo | Discutir el compromiso: `shm` es rápida pero **exige** sincronización explícita |
| **3.** Lectura de un FIFO vacío | `read` **bloquea** (si hay escritores) hasta que haya datos; devuelve `0` (fin de archivo) si todos los escritores cerraron | `ps` con `wchan` mostrando `pipe_read`; `strace` mostrando `read(...) = 0` |
| **4.** Cambio de contexto | Los cambios voluntarios (bloqueo) y no voluntarios (expulsión) tienen causas distintas | `grep ctxt /proc/<PID>/status`; `pidstat -w` |
| **5.** Liberar `shm` antes de terminar | Los segmentos System V **persisten** en el kernel aunque el proceso termine (fuga de recurso del sistema) | `ipcs -m` antes y después; Experimento 8.6 |
| **6.** `shm` frente a FIFO temporales | `shm`: acceso directo, sin sincronización incorporada, acceso aleatorio. FIFO: flujo secuencial, sincronización implícita por bloqueo, cada byte lo consume un único lector | Experimento 8.1 y Experimento 8.2 |

---

## 10. Actividad de extensión

Se propone a los estudiantes, como trabajo posterior a la sesión:

1. Aumentar el buffer compartido a **N posiciones** (arreglo circular en memoria compartida con índices `entrada` y `salida`) y generalizar los semáforos: `vacios` con valor inicial N, `llenos` con valor inicial 0, y un tercer semáforo `mutex` para proteger los índices.
2. Añadir un **segundo productor** y analizar por qué ahora el `mutex` es indispensable.
3. Medir el tiempo total con `time ./lab_ipc` para N = 1, 4 y 16, y explicar las diferencias.
4. Reemplazar los semáforos POSIX por semáforos System V (`semget`, `semop`) y comprobar su existencia con `ipcs -s`.
5. Entregar un breve informe con la salida de `strace -c -f ./lab_ipc` (resumen de llamadas al sistema) y su interpretación.

---

## 11. Referencias

**Bibliografía del laboratorio:**

- Stallings, W. (2018). *Operating Systems: Internals and Design Principles* (9th ed.). Pearson Education.
- Tanenbaum, A. S., & Bos, H. (2015). *Modern Operating Systems* (4th ed.). Pearson Education.
- Kerrisk, M. (2010). *The Linux Programming Interface*. No Starch Press.
- Love, R. (2010). *Linux Kernel Development* (3rd ed.). Addison-Wesley.

**Páginas del manual (`man`) útiles:**

| Tema | Comando |
|---|---|
| FIFO | `man 7 fifo` |
| Tuberías y atomicidad de escritura (`PIPE_BUF`) | `man 7 pipe` |
| Memoria compartida System V | `man 2 shmget`, `man 2 shmat`, `man 2 shmctl`, `man 7 sysvipc` |
| Semáforos POSIX | `man 7 sem_overview`, `man 3 sem_init`, `man 3 sem_wait` |
| Semáforos System V | `man 2 semget`, `man 2 semop` |
| Herramientas | `man ipcs`, `man ipcrm`, `man ps`, `man top`, `man strace`, `man pidstat` |
