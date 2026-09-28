# Actividad Recuperatoria — Unidad 1
## Sistemas Operativos · Gestión de Memoria

> Para estudiantes que **no pudieron presentar** el examen de la Unidad 1 y cuentan con justificación aprobada.

| | |
|---|---|
| **Modalidad** | Individual · taller escrito + práctica de programación en C |
| **Valor** | 100 puntos (equivalentes al examen de la Unidad 1) |
| **Fecha de entrega** | `Septiembre 29 de 2026` |
| **Medio de entrega** | `[aula virtual / diego.loaiza02@usc.edu.co]` |
| **Formato de entrega** | Un `.zip` con `informe.pdf`, `ejercicio1.c`, `ejercicio2.c` y `salidas.txt` |

---

## 1. Objetivos de la actividad

Al terminar, el estudiante habrá demostrado que puede:

1. Identificar los dos roles del sistema operativo (máquina extendida y administrador de recursos) y sus funciones principales.
2. Diferenciar el modo usuario del modo kernel y explicar el papel de las llamadas al sistema en la protección.
3. Comparar las arquitecturas monolítica, en capas y de microkernel, y relacionar la multiprogramación con el reparto de la RAM.
4. Distinguir el particionamiento fijo del dinámico, y la fragmentación interna de la externa, incluyendo las técnicas para mitigarla.
5. Aplicar los algoritmos *first-fit*, *best-fit*, *worst-fit* y *next-fit*, determinando el hueco elegido y el residuo, y valorar su desempeño.
6. Explicar la memoria virtual y traducir direcciones lógicas a físicas con paginación y segmentación, diferenciando *page fault*, *TLB miss* y violación de segmento.
7. Describir los mecanismos de hardware y software de la memoria virtual (MMU, TLB, bits de las entradas de tabla, Copy-on-Write) y el reemplazo de páginas FIFO.
8. Analizar y depurar el código en C de los ejercicios: corregir errores en los algoritmos de colocación y rastrear a mano la traducción de direcciones y el reemplazo FIFO.

## 2. Material de referencia

Es el mismo material usado durante la unidad:

- **Apuntes del curso (HTML):** *Particionamiento de Memoria* y *Gestión de Memoria Virtual*.
- **Plantillas en C:** Ejercicio 1 (particionamiento: first/best/worst/next fit) y Ejercicio 2 (paginación con reemplazo FIFO).
- **Stallings, W.** — *Operating Systems: Internals and Design Principles*, 9.ª ed.: Cap. 2 (visión general del SO), Cap. 7 (gestión de memoria) y Cap. 8 (memoria virtual).
- **Tanenbaum, A. S.** — *Modern Operating Systems*, 4.ª ed.: Cap. 1 (introducción) y Cap. 3 (gestión de memoria).
- **Silberschatz, Galvin, Gagne** — *Operating System Concepts*, 10.ª ed. (lectura complementaria): Cap. 1–2, 9 y 10.

## 3. Instrucciones generales

- El trabajo es **individual**. Puede consultar el material anterior, pero las respuestas y el código deben ser suyos y redactados con sus propias palabras.
- **Justifique** cada respuesta numérica mostrando el procedimiento. Una respuesta correcta sin desarrollo recibe máximo el 50 % del puntaje del ítem.
- El código debe **compilar sin errores** con `gcc -Wall`. Un programa que no compila recibe 0 en el ítem correspondiente.
- No modifique las **firmas** de las funciones de las plantillas ni las funciones ya completas (`main`, `mostrar_particiones`, etc.).
- Incluya al inicio del informe una breve **declaración de autoría** (nombre, código y la frase: "Este trabajo es de mi autoría").

---

## Parte 1 — Conceptos, arquitectura y roles del SO (20 puntos)

**1.1 (4 pts)** Explique los **dos roles** del sistema operativo. Para cada uno dé un ejemplo concreto distinto a los vistos en clase.

**1.2 (4 pts)** Un programa de usuario ejecuta `read()` sobre un archivo. Describa, en pasos numerados, qué ocurre desde que el programa llama a la función hasta que recupera el control. Mencione explícitamente el **bit de modo**, la **llamada al sistema** y el momento en que se cambia entre modo usuario y modo kernel.

**1.3 (4 pts)** Complete la siguiente tabla comparativa (una a dos líneas por celda):

| Aspecto | Monolítico | En capas | Microkernel |
|---|---|---|---|
| ¿Dónde corren los servicios (sistema de archivos, controladores)? | | | |
| Ventaja principal | | | |
| Desventaja principal | | | |

Indique además cómo se clasifica Linux y justifique en una frase.

**1.4 (4 pts)** Explique por qué la **multiprogramación** obliga al SO a decidir cómo repartir la RAM. Distinga con un ejemplo la multiplexación **en el espacio** de la multiplexación **en el tiempo**.

**1.5 (4 pts)** Diga si cada afirmación es verdadera o falsa y **justifique en una línea**:

- a) El intérprete de comandos (*shell*) se ejecuta en modo kernel.
- b) Una función de biblioteca siempre provoca un cambio a modo kernel.
- c) En una arquitectura en capas, la capa N puede usar servicios de capas superiores.
- d) Un microkernel deja en el núcleo solo los servicios mínimos y mueve el resto al espacio de usuario.

---

## Parte 2 — Particionamiento de memoria (20 puntos)

**2.1 (4 pts) Fragmentación.** Un proceso de 170 KB se asigna a una partición fija de 200 KB.

- a) ¿Cuánta fragmentación interna se produce?
- b) Explique con sus palabras por qué el particionamiento **dinámico** sufre fragmentación **externa** y no interna.
- c) Mencione **tres** técnicas que reduzcan la fragmentación externa y diga en una línea cómo actúa cada una.

**2.2 (6 pts) Algoritmos sobre huecos dinámicos.** La lista de huecos libres, en orden de dirección, es:

| Hueco | H1 | H2 | H3 | H4 | H5 | H6 |
|---|---|---|---|---|---|---|
| Tamaño | 120 K | 480 K | 90 K | 350 K | 280 K | 600 K |

Llega **un** proceso de **250 K**. Para *first-fit*, *best-fit* y *worst-fit* indique el hueco elegido y el **residuo** que queda. Presente el resultado en una tabla y comente cuál residuo es más aprovechable y por qué.

**2.3 (6 pts) Simulación con particiones fijas.** Use las particiones del Ejercicio 1 con estos tamaños: `{120, 480, 90, 350, 280, 600}` (índices 0 a 5, todas libres al inicio; cada proceso ocupa **toda** la partición elegida, como en la plantilla). Llegan en este orden los procesos de **130 K, 60 K, 200 K y 400 K**, sin liberarse ninguno.

Para cada algoritmo (*first-fit*, *best-fit*, *worst-fit* y *next-fit*, con `puntero_next_fit = 0`) construya una tabla con: proceso, partición asignada (o `-1`) y fragmentación interna generada. Al final:

- a) Calcule la fragmentación interna **total** de cada algoritmo.
- b) Identifique cuál algoritmo no logra ubicar algún proceso y explique la causa.
- c) Explique en qué se diferencia el resultado de *next-fit* frente a *first-fit* y por qué.

**2.4 (4 pts) Análisis crítico.**

- a) Un compañero afirma que *best-fit* "siempre es el mejor porque desperdicia menos". Refútelo con argumentos.
- b) Un proceso de 100 K termina entre un hueco de 60 K y otro de 140 K. Indique qué queda tras la **fusión de huecos adyacentes** (*coalescing*) y por qué esta operación no aplica al particionamiento fijo.

---

## Parte 3 — Memoria virtual (20 puntos)

**3.1 (8 pts) Traducción con paginación.** Considere direcciones lógicas de **16 bits**, páginas de **4 KB** (12 bits de desplazamiento) y la siguiente tabla de páginas del proceso P1:

| Pág. lógica | Marco físico | Presente | Permisos |
|---|---|---|---|
| 0 | 5 | Sí | R/W |
| 1 | 2 | Sí | Solo lectura |
| 2 | — | No (en swap) | R/W |
| 3 | 6 | Sí | R/W |
| 4 | 0 | Sí | R/W |

Traduzca cada acceso mostrando: separación en página y desplazamiento, consulta a la tabla, verificación de protección y dirección física (en hexadecimal), o el evento que ocurre en su lugar.

| # | Dirección lógica | Tipo de acceso |
|---|---|---|
| a | `0x0B3C` | Lectura |
| b | `0x1200` | Escritura |
| c | `0x2A10` | Lectura |
| d | `0x4FFF` | Lectura |
| e | `0x3001` | Escritura |

Para el acceso **(c)** describa, en pasos, cómo el SO atiende el *page fault*. Para el acceso **(b)** explique por qué la protección de escritura se verifica **antes** que la presencia en RAM.

**3.2 (6 pts) Traducción con segmentación.** Tabla de segmentos del proceso P1:

| Seg. | Nombre | Base | Límite (mayor offset válido) | Permisos |
|---|---|---|---|---|
| 0 | Código | `0x2000` | `0x0FFF` | R/X |
| 1 | Datos | `0x6000` | `0x03FF` | R/W |
| 2 | Pila | `0xA000` | `0x07FF` | R/W |

Resuelva y clasifique cada acceso como válido, violación de segmento o violación de protección:

| # | (segmento, desplazamiento) | Tipo de acceso |
|---|---|---|
| a | (1, `0x3FF`) | Escritura |
| b | (1, `0x400`) | Lectura |
| c | (0, `0x010`) | Escritura |
| d | (2, `0x7FF`) | Lectura |
| e | (3, `0x000`) | Lectura |

**3.3 (6 pts) Conceptos.**

- a) Diferencie un **page fault**, un **TLB miss** y una **excepción de protección**. ¿Cuál de los tres exige leer del swap?
- b) Clasifique como **hardware** o **software**: MMU, TLB, manejador de *page fault*, algoritmo de reemplazo, mapa de marcos libres, bit de presencia.
- c) Explique el bit *dirty* y por qué evita escrituras innecesarias al swap.
- d) Explique cómo funciona *Copy-on-Write* después de un `fork()`.
- e) Compare paginación y segmentación en: tamaño de la unidad, tipo de fragmentación y visibilidad para el programador.

---

## Parte 4 — Análisis y desarrollo de código en C (40 puntos)

### 4.1 Ejercicio 1: particionamiento (14 puntos)

Complete en la plantilla del Ejercicio 1 las funciones `first_fit`, `best_fit`, `worst_fit`, `next_fit` y `liberar_particion`.

- Ejecute el programa con los procesos de la actividad **2.3** y guarde las salidas (`mostrar_particiones` tras cada asignación) en `salidas.txt`.
- Verifique que los resultados coinciden con su simulación a mano. Si difieren, explique el porqué en el informe.

| Criterio | Puntos |
|---|---|
| `first_fit` y `liberar_particion` correctos | 3 |
| `best_fit` y `worst_fit` correctos (incluye inicialización adecuada) | 4 |
| `next_fit` con recorrido circular y actualización de `puntero_next_fit` | 4 |
| Salidas coherentes con la simulación de 2.3 | 3 |

### 4.2 Ejercicio 2: paginación con FIFO (14 puntos)

Complete `cargar_pagina_en_memoria` y `traducir_direccion` en la plantilla del Ejercicio 2 (`TAM_PAGINA = 256`, `NUM_PAGINAS = 8`, `NUM_MARCOS = 4`).

Reemplace la secuencia de ejemplo del `main` por esta y ejecútela con la opción 1 del menú:

```c
int secuencia[] = {100, 600, 1100, 1600, 100, 2000, 700, 300, 1650, 1900};
```

Después:

- a) **Antes de ejecutar**, prediga a mano: página y marco de cada acceso, número de fallos y aciertos, dirección física de la dirección lógica `2000` y el contenido final de cada marco.
- b) Ejecute y compare. Documente cualquier diferencia.
- c) Cambie `NUM_MARCOS` a **3**, vuelva a ejecutar y compare la cantidad de fallos. Explique el resultado.

| Criterio | Puntos |
|---|---|
| `cargar_pagina_en_memoria` con reemplazo FIFO circular e invalidación de la página desalojada | 5 |
| `traducir_direccion` con cálculo de página/desplazamiento, validación de rango, conteo y dirección física | 5 |
| Predicción manual y comparación con la ejecución (a y b) | 2 |
| Análisis de la variación de marcos (c) | 2 |

### 4.3 Depuración de código (12 puntos)

Cada fragmento contiene **un error**. Para cada uno: (i) identifíquelo, (ii) explique qué comportamiento incorrecto produce, con un ejemplo de entrada que lo evidencie, y (iii) escriba la corrección. *(3 pts cada uno)*

**Fragmento A — `worst_fit`**

```c
int worst_fit(Particion particiones[], int n, int tam_proceso) {
    int peor_indice = -1;
    int peor_tamano = 1000000;
    int i;
    for (i = 0; i < n; i++) {
        if (particiones[i].estado == LIBRE &&
            particiones[i].tamano >= tam_proceso &&
            particiones[i].tamano > peor_tamano) {
            peor_indice = i;
            peor_tamano = particiones[i].tamano;
        }
    }
    return peor_indice;
}
```

**Fragmento B — `next_fit`**

```c
int next_fit(Particion particiones[], int n, int tam_proceso) {
    int contador, indice;
    for (contador = 0; contador < n; contador++) {
        indice = (puntero_next_fit + contador) % n;
        if (particiones[indice].estado == LIBRE &&
            particiones[indice].tamano >= tam_proceso) {
            return indice;
        }
    }
    return -1;
}
```

**Fragmento C — `cargar_pagina_en_memoria` (rama de reemplazo)**

```c
marco_elegido = siguiente_marco_a_reemplazar;
siguiente_marco_a_reemplazar =
    (siguiente_marco_a_reemplazar + 1) % NUM_MARCOS;
cola_marcos[marco_elegido] = pagina;
tabla_paginas[pagina] = marco_elegido;
return marco_elegido;
```

**Fragmento D — `traducir_direccion`**

```c
numero_pagina  = direccion_logica % TAM_PAGINA;
desplazamiento = direccion_logica / TAM_PAGINA;
...
direccion_fisica = marco + desplazamiento;
```

---

## 4. Criterios de evaluación

| Parte | Puntos |
|---|---|
| 1. Conceptos, arquitectura y roles | 20 |
| 2. Particionamiento de memoria | 20 |
| 3. Memoria virtual | 20 |
| 4. Análisis y desarrollo de código en C | 40 |
| **Total** | **100** |

**Se valora:** exactitud técnica, procedimiento visible, claridad de la redacción y código legible y comentado.

**Se penaliza:** respuestas sin justificación, código que no compila, copia textual del material o de otras personas.

## 5. Estructura sugerida del informe (`informe.pdf`)

1. Portada y declaración de autoría.
2. Parte 1: respuestas.
3. Parte 2: tablas y análisis.
4. Parte 3: traducciones paso a paso.
5. Parte 4: predicciones manuales, análisis de las salidas y depuración (A–D).
6. Referencias consultadas.

## 6. Recomendaciones

- Repase primero los apuntes de *Particionamiento* y *Memoria virtual*, y luego los capítulos indicados.
- Haga las simulaciones a mano **antes** de ejecutar el código: así detectará sus propios errores de lógica.
- Compile con `gcc -Wall -o ejercicio1 ejercicio1.c` y revise cada advertencia.
- Ante dudas sobre el enunciado, escriba con anticipación a `[correo del docente]`. No se resolverán dudas el día de la entrega.
