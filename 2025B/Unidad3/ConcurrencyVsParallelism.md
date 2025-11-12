# Sistemas Operativos, 2025B

## Paralelismo y Concurrencia en C (Linux)

### 1. Introducción

En la programación moderna, el **paralelismo** y la **concurrencia** son conceptos fundamentales para aprovechar la capacidad de procesamiento de los sistemas multinúcleo. Ambos buscan mejorar la eficiencia y el rendimiento de los programas, pero se diferencian en su enfoque y propósito.

---

### 2. Diferencia entre Concurrencia y Paralelismo

| Concepto | Concurrencia | Paralelismo |
|-----------|---------------|-------------|
| **Definición** | Ejecución *aparente* de múltiples tareas al mismo tiempo. | Ejecución *real* de múltiples tareas al mismo tiempo. |
| **Objetivo** | Mejorar la capacidad de respuesta y gestión de múltiples tareas. | Acelerar la ejecución dividiendo el trabajo entre varios procesadores. |
| **Nivel de abstracción** | Diseño del software (lógica de tareas). | Ejecución física (hardware multinúcleo o multiprocesador). |
| **Ejemplo típico** | Servidor que atiende varias solicitudes alternadamente. | Programa que divide un cálculo pesado entre varios núcleos. |

---

### 3. Tipos de Paralelismo

1. **Paralelismo a nivel de bit**  
   Optimización a nivel de hardware; se procesan varios bits simultáneamente.

2. **Paralelismo a nivel de instrucción (ILP)**  
   El procesador ejecuta múltiples instrucciones al mismo tiempo (pipeline).

3. **Paralelismo a nivel de datos (DLP)**  
   Se aplica la misma operación sobre distintos elementos (vectorización).

4. **Paralelismo a nivel de tareas (TLP)**  
   Distintas tareas o funciones se ejecutan simultáneamente en diferentes núcleos.

---

### 4. Ventajas y Desventajas del Paralelismo

### Ventajas
- Aumento significativo del rendimiento.  
- Reducción del tiempo de ejecución en tareas intensivas.  
- Aprovechamiento de múltiples núcleos.  
- Posibilidad de escalar aplicaciones científicas o de ingeniería.

### Desventajas
- Complejidad en la sincronización y comunicación entre hilos.  
- Posibles condiciones de carrera y bloqueos.  
- Dificultad para depurar errores.  
- Sobrecarga en la creación y gestión de hilos.

---

### 5. Implementación en ANSI C (Linux)

Para implementar paralelismo en C bajo Linux se utiliza la biblioteca **POSIX Threads (pthread)**.

#### Librerías necesarias

```bash
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
```

Compilación:
```bash
gcc programa.c -o programa -lpthread
```

---

### 6. Ejemplo Práctico: Cálculo Paralelo del Promedio

El siguiente programa divide un arreglo en dos mitades y calcula el promedio parcial en cada hilo.

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 8
#define NUM_THREADS 2

int arreglo[N] = {1, 2, 3, 4, 5, 6, 7, 8};
double resultados[NUM_THREADS];

void* calcular_promedio(void* arg) {
    int id = *(int*)arg;
    int inicio = id * (N / NUM_THREADS);
    int fin = inicio + (N / NUM_THREADS);
    double suma = 0;

    for (int i = inicio; i < fin; i++) {
        suma += arreglo[i];
    }
    resultados[id] = suma / (N / NUM_THREADS);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, calcular_promedio, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    double promedio_global = (resultados[0] + resultados[1]) / 2.0;
    printf("Promedio total: %.2f\n", promedio_global);
    pthread_exit(NULL);
}
```

---

### 7. Ejercicio Propuesto

#### Descripción
Modificar el programa anterior para que calcule **la suma total** de un arreglo de 100 números generados aleatoriamente, usando **4 hilos**.

#### Pautas
1. Cambiar `N` a 100 y `NUM_THREADS` a 4.  
2. Cada hilo debe sumar su parte del arreglo.  
3. Guardar las sumas parciales en un arreglo compartido.  
4. El hilo principal debe sumar los resultados y mostrar la suma total.  

#### Preguntas para reflexión
- ¿Qué sucede si dos hilos acceden al mismo índice del arreglo?  
- ¿Cómo podríamos proteger los datos compartidos (mutex)?  
- ¿Cómo escalaría este programa en un equipo de 8 núcleos?  

---

### 8. Conclusiones

El paralelismo es una herramienta poderosa para acelerar el procesamiento de datos y mejorar el rendimiento de los programas, pero requiere un diseño cuidadoso para evitar errores de sincronización. La comprensión de sus diferencias con la concurrencia es esencial para elegir la técnica adecuada según el problema.

---
**Lenguaje:** C (Linux, POSIX Threads)  
**Fecha:** 2025  
