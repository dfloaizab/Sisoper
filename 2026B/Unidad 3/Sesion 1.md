# El kernel y el sistema de archivos

## **Sistemas Operativos 2026B · Universidad Santiago de Cali**
## **Cierre del curso: ¿qué debe tener un kernel moderno?**
<!--**Duración: 2 h 30 min · Incluye: Tema 1, Tema 2, Ejemplos 1 y 2 y el ejercicio E1 (mini `ps`)**-->

---

## 0. Resumen de la sesión

Hasta ahora se estudiaron los **procesos**, la **memoria** y la **comunicación entre procesos**. Todo eso lo hace un solo programa: el **kernel**. En esta sesión se mira por dentro:

1. **Tema 1.** Cómo se organiza un kernel y cómo un programa le pide servicios (**llamadas al sistema**).
2. **Tema 2.** Cómo el kernel organiza los datos en disco y los presenta como **archivos** (inodos, enlaces, descriptores, caché).
3. **Ejercicio E1.** Leer desde C la información que el kernel guarda de cada proceso (su **PCB**) para ver el trabajo de las unidades anteriores desde dentro.

### Objetivos de aprendizaje

Al terminar la sesión, el estudiante podrá:

1. Explicar la diferencia entre **modo usuario** y **modo kernel** y describir qué ocurre cuando un programa hace una llamada al sistema.
2. Comparar las arquitecturas **monolítica, microkernel e híbrida** con un ejemplo de cada una.
3. Explicar qué guarda un **inodo**, en qué se diferencian un **enlace duro** y uno **simbólico**, y qué es un **descriptor de archivo**.
4. Explicar por qué `write` no garantiza que los datos estén en disco y qué hace `fsync`.
5. Leer desde un programa en C el estado de los procesos mediante `/proc`.

### Requisitos

- Linux (nativo, máquina virtual o WSL2) con `gcc`. Opcional: `strace`.
- Compilar con `gcc -Wall -Wextra -std=gnu99 -o ejN ejN.c`. Los ejemplos usan la API POSIX/Linux (la C estándar no incluye `stat`, `link` ni `/proc`).

---

## 1. Cronograma

| Minutos | Bloque | Contenido |
|---|---|---|
| 0-10 | Apertura | Mapa de lo visto y de lo que falta (sección 2) |
| 10-50 | **Tema 1** | Concepto (20 min) + **Ejemplo 1** en vivo (20 min) |
| 50-100 | **Tema 2** | Concepto (20 min) + **Ejemplo 2** (20 min) + discusión (10 min) |
| 100-110 | Descanso | |
| 110-150 | **E1** (arranque) | Exploración de `/proc` a mano y presentación de la plantilla; se termina en casa |

---

## 2. Apertura: ¿qué hace realmente un kernel?

Cada unidad del curso fue una función del kernel:

| Unidad vista | Función del kernel | Llamadas que ya usó |
|---|---|---|
| Procesos y planificación | Crear, planificar y terminar procesos | `fork`, `wait`, `exit` |
| Memoria | Memoria virtual, paginación | `malloc` → `brk`/`mmap` |
| IPC y sincronización | Comunicar y coordinar procesos | `mkfifo`, `shmget`, `sem_wait` |
| **Hoy** | Interfaz de llamadas al sistema y sistema de archivos | `open`, `read`, `write`, `stat` |

Cada llamada de esa tabla es una **llamada al sistema**: el punto donde el programa pide algo que solo el kernel puede hacer. De eso trata el Tema 1.

---

## 3. Tema 1 · Arquitectura del kernel y llamadas al sistema

### 3.1 Dos modos de ejecución

La CPU tiene (al menos) dos niveles de privilegio:

```
        MODO USUARIO                              MODO KERNEL
 ┌───────────────────────────┐            ┌──────────────────────────────┐
 │ Aplicaciones, bibliotecas │            │ Planificador, memoria,       │
 │ (libc), shell, tus        │  syscall   │ sistema de archivos, red,    │
 │ programas en C            │ ─────────▶ │ drivers, seguridad           │
 │                           │ ◀───────── │                              │
 │ NO puede tocar el         │  retorno   │ Acceso total a hardware      │
 │ hardware ni la memoria    │            │ y a toda la memoria          │
 │ de otros procesos         │            │                              │
 └───────────────────────────┘            └──────────────────────────────┘
```

**Por qué existe esta separación:** si cualquier programa pudiera escribir en el disco o en la memoria de otro proceso, un error (o un ataque) podría tumbar el sistema. El hardware impone la regla; el kernel decide qué se permite.

### 3.2 Tres formas de entrar al kernel

| Forma | Quién la provoca | Ejemplo |
|---|---|---|
| **Llamada al sistema** | El programa, a propósito | `read`, `fork`, `open` |
| **Interrupción** | Un dispositivo, de forma asíncrona | Llega un paquete de red; vence el temporizador (base de la planificación expropiativa) |
| **Excepción** | El propio programa, por un error o evento | Fallo de página, división por cero |

### 3.3 Qué pasa en una llamada al sistema

Ejemplo: `write(fd, buf, n)`.

1. El programa llama a la **función de envoltura** de la biblioteca C (`write` de glibc).
2. La envoltura coloca el **número de la llamada** y los argumentos en registros y ejecuta una instrucción especial (`syscall` en x86-64): la CPU pasa a **modo kernel**.
3. El kernel **valida** los argumentos (¿el descriptor es válido? ¿la dirección pertenece al proceso?) y ejecuta el servicio. Si hay que esperar (por ejemplo, un `read` sobre un FIFO vacío), el proceso pasa a **bloqueado** y el planificador ejecuta a otro.
4. El kernel pone el resultado en un registro y **regresa a modo usuario**. Si falló, la envoltura devuelve `-1` y asigna `errno`.

**Consecuencia práctica:** cruzar la frontera cuesta mucho más que una llamada normal a una función (el Ejemplo 1 lo mide). Por eso la biblioteca estándar usa **búferes**: `printf` junta texto y lo envía con pocas llamadas a `write`.

> **Excepción interesante:** algunas funciones de tiempo, como `clock_gettime`, se resuelven en el espacio de usuario mediante el **vDSO** (una pequeña porción de código del kernel mapeada en cada proceso) y no cruzan al kernel. Por eso el Ejemplo 1 puede medir el tiempo sin alterar lo que mide.

### 3.4 Familias de llamadas al sistema

| Familia | Ejemplos | Unidad del curso |
|---|---|---|
| Procesos | `fork`, `execve`, `wait4`, `exit`, `kill` | Procesos |
| Memoria | `brk`, `mmap`, `munmap` | Memoria |
| Archivos y dispositivos | `open`, `read`, `write`, `close`, `stat`, `ioctl` | **Esta sesión** |
| Comunicación | `pipe`, `socket`, `shmget`, `futex` | IPC |
| Información y control | `getpid`, `clock_gettime`, `setrlimit` | Varias |

En Linux hay unas 350 a 450 llamadas, según arquitectura y versión (`man 2 syscalls`).

### 3.5 Arquitecturas de kernel

```
 MONOLÍTICO                    MICROKERNEL                 HÍBRIDO
 ┌─────────────────────┐       ┌─────────────────┐         ┌─────────────────────┐
 │ archivos · red ·    │       │ servidores en   │         │ kernel grande con   │
 │ drivers · memoria · │       │ modo usuario:   │         │ algunos servicios   │
 │ planificador        │       │ archivos, red,  │         │ separados           │
 │ (todo en modo       │       │ drivers         │         │                     │
 │  kernel)            │       ├─────────────────┤         │                     │
 └─────────────────────┘       │ micronúcleo:    │         └─────────────────────┘
                               │ IPC, memoria,   │
                               │ planificación   │
                               └─────────────────┘
```

| Arquitectura | Ventaja | Desventaja | Ejemplos |
|---|---|---|---|
| **Monolítica** | Rápida: los componentes se llaman directamente | Un error en un driver puede tumbar todo el sistema | Linux, BSD |
| **Microkernel** | Aislamiento y robustez; más fácil de verificar | Más mensajes (IPC) entre componentes, mayor costo | Minix 3, QNX, seL4 |
| **Híbrida** | Equilibrio entre ambas | Complejidad | Windows NT, macOS (XNU) |
| Unikernel / exokernel | Mínimo y muy rápido: la aplicación y el kernel forman una sola imagen / el kernel solo reparte hardware | Poco general | MirageOS |

**Linux es monolítico pero modular:** puede cargar y descargar código del kernel en ejecución (**módulos**, p. ej. drivers) sin reiniciar: `lsmod`, `modinfo <módulo>`.

> **Para conectar con el curso:** en un microkernel, los servicios se comunican por **IPC**. Todo lo visto en el laboratorio de FIFO y memoria compartida (costos de copia, sincronización) explica por qué los microkernels pagan un precio en rendimiento.

### 3.6 Ejemplo 1 · ¿Cuánto cuesta cruzar al kernel?

**Qué se mide:** el tiempo de una llamada a una función normal (modo usuario) frente a una llamada al sistema (`getppid`, que casi no hace trabajo, así se mide solo el costo del cruce).

```c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>

static volatile long contador = 0;
static void llamada_usuario(void) { contador++; }   /* costo de una llamada normal */

static double ahora(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    long N = (argc > 1) ? atol(argv[1]) : 1000000;   /* repeticiones */
    double t0, t1, t2;
    long i;

    t0 = ahora();
    for (i = 0; i < N; i++) llamada_usuario();
    t1 = ahora();
    for (i = 0; i < N; i++) syscall(SYS_getppid);   /* cruza a modo kernel */
    t2 = ahora();

    printf("Llamada a funcion de usuario : %7.1f ns\n", (t1 - t0) / (double)N * 1e9);
    printf("Llamada al sistema (getppid) : %7.1f ns\n", (t2 - t1) / (double)N * 1e9);
    printf("Relacion: %.0f veces mas costosa\n", (t2 - t1) / (t1 - t0));
    syscall(SYS_write, 1, "Hola desde write(2) directo\n", 28);
    return 0;
}
```

**Compilar y ejecutar:**

```bash
gcc -Wall -Wextra -std=gnu99 -o ej1 ej1.c
./ej1            # 1 000 000 de repeticiones
./ej1 20000      # menos repeticiones (útil con strace)
```

**Salida de una ejecución de prueba** (los números cambian entre equipos; lo importante es el orden de magnitud):

```
Llamada a funcion de usuario :     2.1 ns
Llamada al sistema (getppid) :   109.2 ns
Relacion: 52 veces mas costosa
Hola desde write(2) directo
```

**Actividades en vivo (20 min):**

1. Ejecute el programa varias veces y anote el rango de la relación. ¿Por qué varía?
2. Si `strace` está disponible, ejecute `strace -c ./ej1 20000` y busque en la tabla la fila de `getppid`: el número de llamadas debe ser aproximadamente `20000` (más unas pocas del arranque del programa). **Nota:** `strace` hace el programa mucho más lento; por eso se reduce `N`.
3. Redirija la salida: `./ej1 > salida.txt; cat salida.txt`. **Observe el orden de las líneas:** con la salida redirigida, «Hola desde write(2) directo» puede aparecer **antes** de las líneas de `printf`.

**Preguntas de discusión:**

1. ¿Por qué el texto de `printf` salió después del de `write`? *(Con la salida redirigida a un archivo, `stdout` queda con búfer completo: `printf` acumula en memoria y vuelca al terminar; `write` va directo al kernel.)* ¿Qué relación tiene con el cuidado de `setvbuf`/`fflush` antes de `fork` en el laboratorio IPC?
2. Si cada llamada al sistema cuesta cientos de nanosegundos, ¿qué conviene hacer al leer un archivo grande: leer byte por byte o de a bloques? (Se comprueba en el ejercicio de bono E2.)
3. ¿Qué arquitectura (monolítica o microkernel) paga más cruces usuario ↔ kernel? ¿Por qué?

---

## 4. Tema 2 · El sistema de archivos

### 4.1 «Todo es un archivo»

En Unix/Linux la interfaz `open/read/write/close` sirve para muchas cosas:

| Qué es | Ruta de ejemplo | Primer carácter de `ls -l` |
|---|---|---|
| Archivo regular | `/etc/passwd` | `-` |
| Directorio | `/tmp` | `d` |
| Enlace simbólico | `/bin` (en muchas distribuciones) | `l` |
| Dispositivo de caracteres | `/dev/null`, `/dev/tty` | `c` |
| Dispositivo de bloques | `/dev/sda` | `b` |
| **FIFO** (el del laboratorio) | `canal_fifo` | `p` |
| Información del kernel | `/proc/<PID>/status` | `-` |

La ventaja: un programa que sabe leer un archivo sabe leer un FIFO, un dispositivo o el estado de un proceso, sin cambiar de interfaz.

### 4.2 Tres tablas: del descriptor al inodo

Cuando un proceso hace `open`, el kernel enlaza tres niveles:

```
 PROCESO A                      KERNEL (global)                     DISCO
┌─────────────────┐       ┌─────────────────────┐          ┌──────────────────┐
│ Tabla de        │       │ Tabla de archivos   │          │ INODO            │
│ descriptores    │       │ abiertos            │          │ tamaño, dueño,   │
│ 0 → stdin       │       │ (posición actual,   │   ───▶   │ permisos, fechas │
│ 1 → stdout      │  ──▶  │  modo, contador)    │          │ bloques de datos │
│ 2 → stderr      │       └─────────────────────┘          └──────────────────┘
│ 3 → (archivo)   │
└─────────────────┘
```

- El **descriptor** (`int`) es un índice en una tabla **por proceso**; los tres primeros son `stdin`, `stdout`, `stderr`.
- Tras `fork`, el hijo **hereda** una copia de la tabla: los dos procesos comparten la misma entrada de la tabla global (y por eso comparten la posición de lectura). Es lo mismo que ocurría con los extremos de un pipe o FIFO en el laboratorio IPC: por eso hay que **cerrar** los extremos que no se usan.
- Se puede ver en vivo: `ls -l /proc/self/fd`.

### 4.3 Inodos

El **inodo** guarda todo lo del archivo **excepto su nombre**:

| Campo | Qué es |
|---|---|
| Número de inodo | Identificador dentro del sistema de archivos |
| Tipo y permisos | Regular, directorio, etc., y los bits `rwxrwxrwx` |
| Dueño y grupo | UID, GID |
| Tamaño | Bytes |
| Fechas | Último acceso, modificación, cambio |
| **Contador de enlaces** | Cuántos nombres apuntan a este inodo |
| Ubicación de los datos | Punteros a los bloques de disco |

Un **directorio** es un archivo especial: una tabla de pares **nombre → número de inodo**. Por eso:

- Dos nombres distintos pueden apuntar al **mismo inodo** (enlace duro).
- Renombrar o mover un archivo dentro del mismo sistema de archivos solo cambia una entrada del directorio, sin copiar datos.
- Borrar un archivo (`unlink`) **quita un nombre y decrementa el contador de enlaces**; los datos se liberan cuando el contador llega a 0 **y** ningún proceso lo tiene abierto.

### 4.4 Enlaces duros y simbólicos

| | Enlace duro | Enlace simbólico |
|---|---|---|
| Qué es | Otro nombre para el **mismo inodo** | Archivo propio cuyo contenido es una **ruta** |
| Inodo | El mismo | Distinto |
| Si se borra el original | Los datos siguen (el contador baja) | El enlace queda **roto** |
| Entre sistemas de archivos | No | Sí |
| Sobre directorios | No (normalmente) | Sí |
| Llamada | `link` | `symlink` |

### 4.5 VFS: una interfaz para muchos sistemas de archivos

El **VFS (Virtual File System)** es una capa del kernel que define una interfaz común (`open`, `read`, `write`, `stat`...). Cada sistema de archivos (ext4, FAT, NFS, `/proc`, tmpfs) implementa esa interfaz. Así, el mismo programa funciona sobre todos.

```
 Aplicación  ──  open / read / write  ──▶  VFS  ──┬─▶ ext4 ──▶ disco
                                                  ├─▶ FAT  ──▶ USB
                                                  ├─▶ NFS  ──▶ red
                                                  └─▶ /proc ─▶ estructuras del kernel
```

### 4.6 Caché de páginas, `write` y `fsync`

- Para no esperar al disco (miles de veces más lento que la memoria), el kernel mantiene una **caché de páginas** en RAM: las lecturas repetidas se sirven desde memoria.
- Las escrituras son **diferidas** (*write-back*): `write` normalmente solo copia los datos a la caché y retorna; el kernel los escribe a disco después.
- **`fsync(fd)`** obliga a escribir a disco los datos de ese descriptor y espera a que termine. Es más lento, pero es la garantía real de que los datos sobreviven a un apagón.
- **Journaling:** el sistema registra primero los cambios en un **diario** y luego los aplica, de modo que tras una caída puede terminar o deshacer lo que quedó a medias, sin recorrer todo el disco.

| Pregunta | Respuesta |
|---|---|
| ¿`write` retornó 0 errores? | Los datos están en la caché del kernel |
| ¿Están en el disco? | Solo tras `fsync` (o cuando el kernel los escriba por su cuenta) |
| ¿Por qué la segunda lectura de un archivo es más rápida? | Se sirve desde la caché de páginas |

### 4.7 Ejemplo 2 · Inodos, enlaces duros y simbólicos

**Qué se observa:** número de inodo, contador de enlaces, tamaño y permisos de un archivo y de sus enlaces; qué cambia al borrar el original.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

static void describir(const char *ruta)
{
    struct stat st;
    const char *tipo;
    if (lstat(ruta, &st) == -1) { perror(ruta); return; }
    if (S_ISREG(st.st_mode)) tipo = "archivo regular";
    else if (S_ISDIR(st.st_mode)) tipo = "directorio";
    else if (S_ISLNK(st.st_mode)) tipo = "enlace simbolico";
    else if (S_ISFIFO(st.st_mode)) tipo = "FIFO";
    else if (S_ISCHR(st.st_mode)) tipo = "dispositivo de caracteres";
    else tipo = "otro";
    printf("%-14s inodo=%-8lu enlaces=%lu tamano=%ld bytes bloques=%ld permisos=%o (%s)\n",
           ruta, (unsigned long)st.st_ino, (unsigned long)st.st_nlink,
           (long)st.st_size, (long)st.st_blocks, (unsigned)(st.st_mode & 0777), tipo);
}

int main(void)
{
    int fd = open("original.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("open"); return 1; }
    write(fd, "contenido\n", 10);
    close(fd);

    unlink("duro.txt"); unlink("simbolico.txt");
    if (link("original.txt", "duro.txt") == -1) perror("link");
    if (symlink("original.txt", "simbolico.txt") == -1) perror("symlink");

    describir("original.txt");
    describir("duro.txt");        /* MISMO inodo, enlaces=2 */
    describir("simbolico.txt");   /* inodo distinto: guarda una ruta */
    describir("/dev/null");
    describir("/tmp");

    unlink("original.txt");
    printf("-- tras borrar original.txt --\n");
    describir("duro.txt");        /* los datos siguen: enlaces=1 */
    if (access("simbolico.txt", F_OK) == -1) printf("simbolico.txt: enlace roto\n");
    unlink("duro.txt"); unlink("simbolico.txt");
    return 0;
}
```

**Salida de una ejecución de prueba** (los números de inodo cambian en cada equipo):

```
original.txt   inodo=434202   enlaces=2 tamano=10 bytes bloques=8 permisos=644 (archivo regular)
duro.txt       inodo=434202   enlaces=2 tamano=10 bytes bloques=8 permisos=644 (archivo regular)
simbolico.txt  inodo=434203   enlaces=1 tamano=12 bytes bloques=0 permisos=777 (enlace simbolico)
/dev/null      inodo=3        enlaces=1 tamano=0 bytes bloques=0 permisos=666 (dispositivo de caracteres)
/tmp           inodo=20       enlaces=4 tamano=4096 bytes bloques=8 permisos=777 (directorio)
-- tras borrar original.txt --
duro.txt       inodo=434202   enlaces=1 tamano=10 bytes bloques=8 permisos=644 (archivo regular)
simbolico.txt: enlace roto
```

**Cómo leer la salida:**

| Observación | Explicación |
|---|---|
| `original.txt` y `duro.txt` tienen el **mismo inodo** y `enlaces=2` | Son dos nombres para los mismos datos |
| `simbolico.txt` tiene otro inodo y `tamano=12` | Guarda la cadena `original.txt` (12 caracteres) y ocupa 0 bloques de datos |
| `permisos=777` del enlace simbólico | Los permisos reales son los del destino |
| `/dev/null` con `tamano=0` y tipo de dispositivo | Es un archivo especial: no hay datos en disco |
| Tras borrar `original.txt`, `duro.txt` conserva todo y `enlaces=1` | Se quitó un nombre; los datos siguen mientras quede otro nombre |
| `simbolico.txt` queda roto | Apuntaba a un nombre que ya no existe |

**Actividades en vivo (20 min):**

1. Compile y ejecute. Antes de ver la salida, pida a los estudiantes que **predigan** el número de enlaces de cada línea.
2. En la terminal, sin programa: `echo hola > a.txt; ln a.txt b.txt; ln -s a.txt c.txt; ls -li` y luego `rm a.txt; ls -li; cat b.txt; cat c.txt`.
3. `ls -l /proc/self/fd` y `ls -l /dev/null /tmp /bin`: identifique el tipo por el primer carácter.
4. Explore `stat original.txt` (si el archivo aún existe) y compare con la salida del programa.

**Preguntas de discusión (10 min):**

1. ¿Por qué un enlace duro no puede cruzar sistemas de archivos? *(Los números de inodo solo tienen sentido dentro de un sistema de archivos.)*
2. Si un proceso tiene un archivo abierto y otro proceso lo borra con `rm`, ¿el primero puede seguir leyendo? ¿Cuándo se liberan los datos? *(Sí; cuando se cierra el último descriptor y el contador de enlaces es 0.)*
3. En el laboratorio IPC, ¿qué pasaba con el archivo `canal_fifo` si el programa terminaba sin `unlink`? ¿Qué muestra `stat canal_fifo`?
4. Un programa hace `write` y se va la luz antes de `fsync`. ¿Qué puede pasar con los datos?

---

## 5. Descanso (10 min)

---

## 6. E1 · Mini `ps`: leer el PCB desde `/proc`

**Objetivo:** ver los datos del **PCB** de cada proceso desde un programa en C, y conectar con las unidades de procesos y planificación (estados, cambios de contexto voluntarios e involuntarios).

### 6.1 ¿Qué es `/proc`?

`/proc` es un sistema de archivos **virtual**: no existe en el disco; el kernel genera su contenido cuando se lee. Cada proceso tiene una carpeta `/proc/<PID>/`. Es el VFS en acción: un programa usa `fopen` y `fgets` para leer **estructuras del kernel**.

### 6.2 Exploración a mano (10 min, en clase)

```bash
echo $$                                  # PID de su shell
cat /proc/$$/status | head -20           # nombre, estado, PID, PPid, memoria, cambios de contexto...
ls -l /proc/$$/fd                        # descriptores abiertos de su shell
grep ctxt /proc/$$/status                # cambios de contexto voluntarios e involuntarios
```

Campos de `status` que usará el ejercicio:

| Línea | Qué es | Relación con el curso |
|---|---|---|
| `Name:` | Nombre del programa | PCB: identificación |
| `State:` | `R` (ejecutando/listo), `S` (dormido), `D`, `T`, `Z` (zombi), `I` (inactivo, hilos del kernel) | Estados del proceso |
| `PPid:` | PID del padre | Jerarquía de procesos |
| `VmRSS:` | Memoria física residente (KiB) | Memoria virtual: lo que realmente está en RAM |
| `voluntary_ctxt_switches:` | Veces que el proceso cedió la CPU porque se bloqueó | Planificación |
| `nonvoluntary_ctxt_switches:` | Veces que el planificador lo expulsó | Planificación (quantum) |

### 6.3 Enunciado

Escriba `minips.c` que recorra `/proc` y, para cada proceso, muestre **PID, PPID, nombre, estado, memoria residente (VmRSS en KiB), cambios de contexto voluntarios e involuntarios**. Al final debe mostrar el **total de procesos** y la **suma de memoria residente**.

**Formato sugerido de salida:**

```
   PID   PPID NOMBRE           E  RSS(KiB)      VOL      INV
     1      0 systemd          S     12480      523       41
   ...
Procesos: 187 | RSS total: 1456320 KiB
```

### 6.4 Requisitos

1. Ignorar las entradas de `/proc` cuyo nombre no sea numérico.
2. Tolerar que un proceso **desaparezca** entre el momento de listar y el de abrir su `status` (omitirlo sin fallar).
3. Los hilos del kernel no tienen línea `VmRSS`: deben mostrarse con 0.
4. Verificar los retornos de las llamadas, cerrar `DIR` y `FILE`, y compilar sin advertencias con `-Wall -Wextra`.

### 6.5 Pistas

| Necesita | Use | Consulte |
|---|---|---|
| Recorrer un directorio | `opendir`, `readdir`, `closedir` | `man 3 readdir` |
| Saber si un nombre es numérico | `isdigit` sobre el primer carácter | `man 3 isdigit` |
| Construir la ruta | `snprintf(ruta, sizeof ruta, "/proc/%d/status", pid)` | |
| Leer línea por línea | `fopen`, `fgets` | |
| Extraer un valor de una línea | `sscanf(linea, "PPid: %d", &p)` (devuelve 1 si coincidió) | `man 3 sscanf` |
| Estructura de `/proc` | `man 5 proc` | |

### 6.6 Plantilla para completar

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

typedef struct {
    int pid, ppid;
    char nombre[64];
    char estado;
    long rss_kb;
    long cs_vol, cs_inv;
} InfoProc;

/* Lee /proc/<pid>/status y llena *p. Retorna 0 si pudo, -1 si el proceso ya no existe. */
static int leer_proceso(int pid, InfoProc *p)
{
    /* TODO 1: poner todo en cero (memset) y guardar el pid
     * TODO 2: construir la ruta /proc/<pid>/status con snprintf y abrirla con fopen
     *         (si falla, retornar -1: el proceso pudo terminar)
     * TODO 3: leer linea por linea (fgets) y extraer con sscanf los campos:
     *         Name, State, PPid, VmRSS, voluntary_ctxt_switches, nonvoluntary_ctxt_switches
     * TODO 4: cerrar el archivo y retornar 0 */
    return -1;
}

int main(void)
{
    /* TODO 5: abrir /proc con opendir y verificar el retorno
     * TODO 6: imprimir el encabezado; luego, por cada entrada (readdir) cuyo nombre
     *         empiece con digito, llamar a leer_proceso(atoi(nombre), &p) e imprimir una fila
     * TODO 7: acumular el total de procesos y de RSS; imprimir el resumen
     * TODO 8: cerrar el directorio */
    return 0;
}
```

### 6.7 Preguntas de análisis (se entregan con el código)

1. ¿Qué estado predomina (`S`, `R`, `I`, `Z`)? ¿Por qué casi ninguno aparece en `R`?
2. ¿Qué procesos acumulan más cambios **involuntarios**? ¿Y **voluntarios**? Relacione con procesos limitados por CPU y por E/S.
3. Ejecute `./lab_ipc` (laboratorio anterior) en otra terminal y localice a sus tres hijos en el listado. ¿En qué estado están y por qué?
4. ¿Por qué la suma de `VmRSS` sobrestima la memoria física realmente usada? *(Las páginas compartidas —bibliotecas, copy-on-write— se cuentan en cada proceso.)*

### 6.8 Entrega y rúbrica (100 puntos)

**Entrega:** `minips.c` + una página con la salida de una ejecución y las respuestas a las preguntas, **al inicio de la Sesión 2**.

| Criterio | Puntos |
|---|---|
| Funciona: lista los procesos con todos los campos y el resumen final | 40 |
| Robustez: ignora entradas no numéricas, tolera procesos que desaparecen, verifica retornos | 25 |
| Recursos: cierra `DIR` y `FILE`, compila sin advertencias | 15 |
| Preguntas de análisis respondidas con evidencia de su propia salida | 20 |

**Si la clase va corta de tiempo:** entregue la función `leer_proceso` ya resuelta y pida solo el `main`, o al contrario. Cualquiera de las dos mitades se resuelve en una hora de trabajo en casa.

---

## 7. Bono E2 · Copiar archivos con llamadas al sistema y medir el efecto del búfer

**Objetivo:** comprobar con medidas el costo de las llamadas al sistema (Ejemplo 1) y el papel de la caché y de `fsync` (Tema 2).

**Enunciado:** escriba `copiar.c` que reciba `origen destino tam_buffer` y copie el archivo usando **solo** `open`, `read`, `write` y `close`, con un búfer de `tam_buffer` bytes reservado con `malloc`. Maneje lecturas y escrituras parciales y verifique los retornos.

1. Cree un archivo de prueba (por ejemplo, `dd if=/dev/urandom of=prueba.bin bs=1M count=100`).
2. Mida con `time` la copia para búferes de **1, 64, 4096, 65536 y 1048576** bytes. Con 1 byte use un archivo pequeño (por ejemplo, 1 MiB): será muy lenta. Tabule tiempo y número de llamadas (`strace -c`, si está disponible).
3. Repita el mejor caso agregando `fsync(fd_destino)` antes de cerrar y compare.
4. Verifique con `cmp origen destino` que la copia es idéntica.

**Resultado esperado (cualitativo):** el tiempo cae drásticamente al pasar de 1 byte a unos pocos KiB y luego se estabiliza; `fsync` añade el costo de escribir realmente a disco.

---

## 8. Autoevaluación de cierre (5 minutos, opcional)

<details>
<summary>1. ¿Qué diferencia una llamada al sistema de una llamada a una función normal?</summary>
La llamada al sistema cambia a modo kernel (instrucción especial, validación de argumentos) y regresa; por eso cuesta mucho más.
</details>

<details>
<summary>2. ¿Por qué Linux, siendo monolítico, puede cargar drivers sin reiniciar?</summary>
Porque admite módulos cargables: código que se enlaza al kernel en ejecución (<code>lsmod</code>, <code>modinfo</code>).
</details>

<details>
<summary>3. ¿Qué guarda el inodo y qué NO guarda?</summary>
Guarda tipo, permisos, dueño, tamaño, fechas, contador de enlaces y ubicación de los datos. No guarda el nombre: este vive en el directorio.
</details>

<details>
<summary>4. Un archivo tiene <code>enlaces=2</code>. Se borra uno de sus nombres. ¿Qué pasa con los datos?</summary>
Se conservan: el contador baja a 1 y el otro nombre sigue apuntando al mismo inodo.
</details>

<details>
<summary>5. ¿Qué garantiza <code>fsync</code> que <code>write</code> no?</summary>
Que los datos del descriptor se escribieron a disco; <code>write</code> normalmente solo los deja en la caché de páginas.
</details>

---

## 9. Solución de referencia de E1 (para el docente)

<details>
<summary><strong>Ver solución</strong></summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

typedef struct {
    int pid, ppid;
    char nombre[64];
    char estado;
    long rss_kb;
    long cs_vol, cs_inv;
} InfoProc;

static int leer_proceso(int pid, InfoProc *p)
{
    char ruta[64], linea[256];
    FILE *f;

    memset(p, 0, sizeof *p);
    p->pid = pid;
    snprintf(ruta, sizeof ruta, "/proc/%d/status", pid);
    f = fopen(ruta, "r");
    if (!f) return -1;                    /* pudo terminar mientras listabamos */
    while (fgets(linea, sizeof linea, f)) {
        if (sscanf(linea, "Name: %63s", p->nombre) == 1) continue;
        if (sscanf(linea, "State: %c", &p->estado) == 1) continue;
        if (sscanf(linea, "PPid: %d", &p->ppid) == 1) continue;
        if (sscanf(linea, "VmRSS: %ld", &p->rss_kb) == 1) continue;
        if (sscanf(linea, "voluntary_ctxt_switches: %ld", &p->cs_vol) == 1) continue;
        if (sscanf(linea, "nonvoluntary_ctxt_switches: %ld", &p->cs_inv) == 1) continue;
    }
    fclose(f);
    return 0;
}

int main(void)
{
    DIR *d = opendir("/proc");
    struct dirent *e;
    InfoProc p;
    int total = 0;
    long rss_total = 0;

    if (!d) { perror("opendir /proc"); return 1; }
    printf("%6s %6s %-16s %c %9s %8s %8s\n", "PID", "PPID", "NOMBRE", 'E', "RSS(KiB)", "VOL", "INV");
    while ((e = readdir(d)) != NULL) {
        if (!isdigit((unsigned char)e->d_name[0])) continue;    /* solo carpetas numericas */
        if (leer_proceso(atoi(e->d_name), &p) == -1) continue;
        printf("%6d %6d %-16s %c %9ld %8ld %8ld\n", p.pid, p.ppid, p.nombre, p.estado,
               p.rss_kb, p.cs_vol, p.cs_inv);
        total++;
        rss_total += p.rss_kb;
    }
    closedir(d);
    printf("Procesos: %d | RSS total: %ld KiB\n", total, rss_total);
    return 0;
}
```

**Notas para la revisión:**

- `%63s` en `Name` solo captura la primera palabra y evita desbordar el arreglo `nombre` (de 64). Los nombres con espacios se truncan; es aceptable en este ejercicio.
- Los hilos del kernel (`kthreadd`, `kworker`) no tienen línea `VmRSS`; quedan en 0 gracias al `memset`.
- `if (!f) return -1;` implementa el requisito de tolerar procesos que terminan mientras se listan.
- Errores frecuentes: no cerrar `DIR`/`FILE`, desbordar `nombre` con `%s` sin ancho, usar `atoi` sobre entradas no numéricas sin filtrar, y mezclar `printf` con formato incorrecto para `long` (`%ld`).
- Probada en Linux: el listado empieza con el PID 1 y aparecen los procesos entre corchetes del kernel.

</details>

---

## 10. Referencias

- Stallings, W. (2018). *Operating Systems: Internals and Design Principles* (9th ed.). Pearson.
- Tanenbaum, A. S., & Bos, H. (2015). *Modern Operating Systems* (4th ed.). Pearson.
- Kerrisk, M. (2010). *The Linux Programming Interface*. No Starch Press.
- Love, R. (2010). *Linux Kernel Development* (3rd ed.). Addison-Wesley.
- Páginas del manual: `syscalls(2)`, `stat(2)`, `link(2)`, `symlink(2)`, `open(2)`, `fsync(2)`, `proc(5)`, `vdso(7)`, `inode(7)`.
