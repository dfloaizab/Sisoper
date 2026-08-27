# Unidad 1 — Gestión de Memoria: Arreglos Dinámicos en ANSI C

## Objetivos de aprendizaje

Al finalizar este ejercicio, el estudiante estará en capacidad de:

1. Identificar y explicar los **syscalls** que el sistema operativo expone (directa o indirectamente, vía la biblioteca estándar de C) para la gestión de memoria dinámica, entendiendo al SO como una **capa de abstracción** entre el proceso y el hardware de memoria físico.
2. Comprender **cómo gestiona el SO la memoria** cuando un proceso solicita más espacio del que ya tiene reservado (crecimiento del heap, posible reubicación de bloques, fragmentación).
3. Implementar en ANSI C un **arreglo dinámico** (dynamic array / vector) que crece automáticamente usando `malloc`, `realloc` y `free` correctamente, evitando fugas de memoria.

## Contexto teórico previo

### El SO como capa de abstracción

Cuando un programa en C llama a `malloc()`, en realidad no está hablando directamente con el hardware ni pidiéndole memoria física a la RAM. Ocurre lo siguiente:

```
Programa en C
     │  malloc(n)
     ▼
Biblioteca estándar de C (glibc / libc)
     │  administra un "heap" en espacio de usuario;
     │  sólo llama al SO cuando necesita más memoria virtual
     ▼
Syscalls: brk() / sbrk()  ó  mmap() / munmap()
     ▼
Núcleo del Sistema Operativo
     │  actualiza las tablas de páginas del proceso
     │  (memoria virtual), sin comprometer RAM física
     │  hasta que la página se toca por primera vez (page fault)
     ▼
Hardware: MMU (Memory Management Unit) + RAM física
```

Puntos clave para discutir en clase:

- `malloc`, `realloc` y `free` **no son syscalls**: son funciones de la biblioteca estándar de C que administran un *heap* en espacio de usuario y que, bajo el capó, invocan syscalls sólo quando es necesario.
- Los syscalls reales involucrados normalmente son:
  - **`brk()` / `sbrk()`**: mueven el "program break" (el límite superior del heap) para pedir o liberar bloques grandes y contiguos de memoria virtual. Es el mecanismo clásico y más simple.
  - **`mmap()` / `munmap()`**: reservan (o liberan) regiones de memoria virtual de forma independiente al heap tradicional, típicamente usadas por glibc para solicitudes grandes (por defecto, `malloc` usa `mmap` para bloques mayores a ~128 KB, el llamado `M_MMAP_THRESHOLD`).
- El SO entrega **memoria virtual**, no memoria física directamente. La asignación física real (RAM) ocurre de forma perezosa (*lazy allocation*), disparada por un **page fault** la primera vez que el proceso escribe en esa página.
- Esto explica por qué `malloc` de un arreglo enorme puede "tener éxito" instantáneamente aunque el sistema no tenga tanta RAM libre en ese momento (overcommit).

### ¿Qué pasa cuando pido *mucho más* espacio? (realloc y el crecimiento del arreglo)

Cuando se llama `realloc(ptr, nuevo_tamaño)`, el SO/la biblioteca de C puede hacer una de dos cosas:

1. **Extender in-place**: si hay espacio libre contiguo justo después del bloque actual (por ejemplo, porque el heap termina ahí y puede mover el `brk`, o porque el bloque vecino en el heap estaba libre), simplemente extiende el bloque. No hay copia de datos.
2. **Reubicar (mover) el bloque**: si no hay espacio contiguo suficiente, `realloc` debe:
   - Buscar/pedir un bloque nuevo lo bastante grande (posiblemente vía `sbrk`/`mmap`).
   - **Copiar** todos los datos del bloque viejo al nuevo.
   - Liberar el bloque viejo.
   - Devolver un puntero **distinto** al original.

Esto tiene una consecuencia práctica muy importante: **nunca se debe asumir que el puntero devuelto por `realloc` es el mismo**, y **nunca se debe sobrescribir el puntero original antes de verificar que `realloc` no devolvió `NULL`** (si devuelve `NULL`, el bloque original sigue siendo válido y se perdería su referencia).

## Requisitos previos

- Compilador `gcc` en Linux (o WSL en Windows).
- Conocimientos básicos de punteros y structs en C.
- (Opcional, para la parte de observación de syscalls) tener instalado `strace`:
  ```bash
  sudo apt-get install strace
  ```

## Paso a paso del ejercicio

### Paso 1 — Preparar el entorno de trabajo

```bash
mkdir arreglo_dinamico
cd arreglo_dinamico
touch arreglo_dinamico.c
```

### Paso 2 — Definir la estructura del arreglo dinámico

Un arreglo dinámico "inteligente" no solo guarda el puntero a los datos: también necesita saber cuántos elementos tiene actualmente (`size`) y cuánta capacidad reservada tiene (`capacity`), para saber cuándo debe crecer.

```c
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *datos;
    size_t size;      /* elementos actualmente usados */
    size_t capacity;  /* elementos que caben sin pedir más memoria */
} ArregloDinamico;
```

### Paso 3 — Función de inicialización (`malloc`)

```c
void arreglo_iniciar(ArregloDinamico *arr, size_t capacidad_inicial) {
    arr->datos = (int *) malloc(capacidad_inicial * sizeof(int));
    if (arr->datos == NULL) {
        fprintf(stderr, "Error: malloc no pudo reservar memoria.\n");
        exit(EXIT_FAILURE);
    }
    arr->size = 0;
    arr->capacity = capacidad_inicial;
    printf("[INFO] Arreglo iniciado con capacidad para %zu elementos.\n", capacidad_inicial);
}
```

**Discusión en clase:** ¿qué pasaría si `capacidad_inicial` fuera 0? ¿Es válido `malloc(0)`? (Sí, pero el comportamiento del puntero devuelto es dependiente de la implementación: puede ser `NULL` o un puntero único no desreferenciable).

### Paso 4 — Función para agregar un elemento, creciendo con `realloc` cuando haga falta

Esta es la parte central del ejercicio: cuando el arreglo se llena, se duplica su capacidad (estrategia de *crecimiento geométrico*, la misma que usan internamente `std::vector` de C++ o `ArrayList` de Java).

```c
void arreglo_agregar(ArregloDinamico *arr, int valor) {
    if (arr->size == arr->capacity) {
        size_t nueva_capacidad = arr->capacity * 2;
        printf("[INFO] Capacidad agotada (%zu/%zu). Solicitando realloc a %zu elementos...\n",
               arr->size, arr->capacity, nueva_capacidad);

        int *tmp = (int *) realloc(arr->datos, nueva_capacidad * sizeof(int));
        if (tmp == NULL) {
            fprintf(stderr, "Error: realloc no pudo ampliar la memoria.\n");
            free(arr->datos);
            exit(EXIT_FAILURE);
        }

        arr->datos = tmp;
        arr->capacity = nueva_capacidad;
        printf("[INFO] Realloc exitoso. Nueva capacidad: %zu elementos.\n", arr->capacity);
    }

    arr->datos[arr->size] = valor;
    arr->size++;
}
```

**Por qué duplicar y no sumar de a uno:** si cada `arreglo_agregar` pidiera exactamente un elemento más (`capacity + 1`), tendríamos una llamada a `realloc` por cada inserción — costosísimo, porque cada `realloc` puede implicar copiar todo el arreglo. Duplicando la capacidad, el número de `realloc` necesarios para insertar *n* elementos es sólo O(log n), y el costo total amortizado por inserción es O(1).

### Paso 5 — Función de liberación (`free`)

```c
void arreglo_liberar(ArregloDinamico *arr) {
    free(arr->datos);
    arr->datos = NULL;   /* evita punteros colgantes (dangling pointers) */
    arr->size = 0;
    arr->capacity = 0;
    printf("[INFO] Memoria liberada correctamente.\n");
}
```

### Paso 6 — Función `main` de prueba

```c
int main(void) {
    ArregloDinamico arr;
    arreglo_iniciar(&arr, 2); /* capacidad inicial deliberadamente pequeña
                                  para forzar varios realloc y observarlos */

    for (int i = 1; i <= 20; i++) {
        arreglo_agregar(&arr, i * 10);
    }

    printf("\nContenido final del arreglo (%zu elementos, capacidad %zu):\n",
           arr.size, arr.capacity);
    for (size_t i = 0; i < arr.size; i++) {
        printf("%d ", arr.datos[i]);
    }
    printf("\n");

    arreglo_liberar(&arr);
    return 0;
}
```

### Paso 7 — Compilar y ejecutar

```bash
gcc -Wall -Wextra -g -o arreglo_dinamico arreglo_dinamico.c
./arreglo_dinamico
```

Se debe observar en la consola cómo la capacidad va duplicándose (2 → 4 → 8 → 16 → 32) cada vez que `size` alcanza a `capacity`, mucho antes de llegar a los 20 elementos insertados.

### Paso 8 — Observar los syscalls reales con `strace`

Este paso conecta directamente la implementación con el Objetivo 1 (el SO como capa de abstracción): permite **ver** los syscalls que la biblioteca de C dispara por debajo de `malloc`/`realloc`/`free`.

```bash
strace -e trace=brk,mmap,munmap ./arreglo_dinamico
```

Preguntas guía para el análisis de la salida:

- ¿Cuántas llamadas a `brk` o `mmap` aparecen frente a cuántas llamadas a `arreglo_agregar` se hicieron en el código? (Deberían ser muchas menos syscalls que llamadas a `arreglo_agregar`, evidencia de que la biblioteca de C actúa como intermediaria y no pide memoria al SO en cada `malloc`/`realloc`).
- ¿Aparece algún `mmap` en vez de `brk`? ¿Para qué tamaños de bloque ocurre esto?
- ¿En qué momento del programa aparece `munmap` o el `brk` decreciente (si ocurre)?

### Paso 9 — Verificar fugas de memoria con `valgrind` (opcional pero recomendado)

```bash
sudo apt-get install valgrind
valgrind --leak-check=full ./arreglo_dinamico
```

Debe reportar `0 bytes lost` si la función `arreglo_liberar` fue llamada correctamente. Se recomienda comentar temporalmente la llamada a `arreglo_liberar(&arr);` para que el estudiante vea cómo `valgrind` sí detecta la fuga, y entienda la diferencia.

## Ejercicio de extensión (para después de esta sesión)

Amplíe el programa desarrollado hoy agregando las siguientes funcionalidades, manteniendo la misma estructura `ArregloDinamico`:

1. **`arreglo_insertar(ArregloDinamico *arr, size_t indice, int valor)`**: inserta un valor en una posición específica, desplazando los elementos posteriores una posición a la derecha (recordar que puede requerir `realloc` primero).
2. **`arreglo_eliminar(ArregloDinamico *arr, size_t indice)`**: elimina el elemento en una posición dada, desplazando los elementos posteriores una posición a la izquierda. Como reto adicional, implemente una política de **reducción de capacidad** (shrink-to-fit): si `size` cae por debajo de, por ejemplo, 1/4 de `capacity`, llame a `realloc` para reducir el bloque a la mitad, y mida con `strace` si el SO efectivamente devuelve memoria (pista: con `brk` sí puede ocurrir si el bloque está al final del heap; con bloques manejados por `mmap`, `munmap` libera la región completa).
3. **`arreglo_buscar(ArregloDinamico *arr, int valor)`**: retorna el índice de la primera ocurrencia de un valor, o -1 si no existe.
4. **Generalizar el tipo de dato**: convierta `ArregloDinamico` en un arreglo dinámico genérico usando `void *` y un parámetro `size_t elem_size`, de forma que pueda almacenar `int`, `float`, o `struct` arbitrarios (esto obliga a usar aritmética de punteros con `char *` internamente, ya que no se puede indexar directamente un `void *`).
5. **Comparar con `mmap` explícito**: reescriba la función `arreglo_iniciar` para que, en vez de usar `malloc`, reserve la memoria directamente con `mmap(NULL, tamaño, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)`, y libere con `munmap`. Compare con `strace` las diferencias de comportamiento frente a la versión basada en `malloc`.
6. Escriba un pequeño informe (media página) respondiendo: *¿por qué el sistema operativo prefiere no dar memoria física inmediatamente al hacer `malloc`, sino que espera hasta el primer acceso (page fault)?*

## Bibliografía

- Silberschatz, A., Galvin, P. B., & Gagne, G. (2018). *Operating System Concepts* (10th ed.). Wiley. — Capítulos sobre gestión de memoria y memoria virtual.
- Bryant, R. E., & O'Hallaron, D. R. (2015). *Computer Systems: A Programmer's Perspective* (3rd ed.). Pearson. — Capítulo 9, "Virtual Memory", incluye una explicación detallada de `malloc`/`free` y asignadores dinámicos.
- Kernighan, B. W., & Ritchie, D. M. (1988). *The C Programming Language* (2nd ed.). Prentice Hall.
- Love, R. (2010). *Linux Kernel Development* (3rd ed.). Addison-Wesley. — Capítulo sobre gestión de memoria del kernel de Linux.
- Documentación oficial de Linux (man pages): `man malloc`, `man realloc`, `man free`, `man brk`, `man mmap`, `man strace`, `man valgrind`.
- Drepper, U. (2007). *Memory part 2: CPU caches* y la serie *What Every Programmer Should Know About Memory*. — Referencia técnica sobre el comportamiento real de los asignadores de memoria en glibc.
- Gorman, M. (2004). *Understanding the Linux Virtual Memory Manager*. Prentice Hall.
