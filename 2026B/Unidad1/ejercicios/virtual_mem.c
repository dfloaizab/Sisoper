/* ============================================================================
 * SISTEMAS OPERATIVOS
 * Ejercicio 2 (PLANTILLA): Memoria virtual - Paginacion
 * Traduccion de direcciones logicas a fisicas y reemplazo de paginas (FIFO)
 *
 * INSTRUCCIONES:
 * Complete las funciones marcadas con "TODO". No modifique las firmas.
 *
 * MODELO SIMULADO:
 * - El espacio logico del proceso tiene NUM_PAGINAS paginas.
 * - La memoria fisica tiene NUM_MARCOS marcos (frames), y NUM_MARCOS <
 *   NUM_PAGINAS, por lo que NO todas las paginas caben en memoria a la vez.
 * - TAM_PAGINA define el tamano (en bytes) de cada pagina/marco.
 * - Una direccion logica se descompone asi:
 *       numero_pagina = direccion_logica / TAM_PAGINA
 *       desplazamiento = direccion_logica % TAM_PAGINA
 * - La tabla de paginas indica, para cada pagina logica, en que marco
 *   fisico esta cargada (o -1 si no esta cargada: FALLO DE PAGINA).
 * - Cuando ocurre un fallo de pagina y todos los marcos estan ocupados,
 *   se debe reemplazar la pagina que lleva MAS TIEMPO en memoria
 *   (algoritmo FIFO), usando una cola de marcos.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#define TAM_PAGINA   256   /* bytes por pagina/marco               */
#define NUM_PAGINAS  8     /* paginas logicas del proceso           */
#define NUM_MARCOS   4     /* marcos fisicos disponibles (< NUM_PAGINAS) */

/* Tabla de paginas: tabla_paginas[i] = marco fisico donde esta cargada
 * la pagina logica i, o -1 si la pagina no esta actualmente en memoria. */
int tabla_paginas[NUM_PAGINAS];

/* Cola FIFO de marcos: guarda, en orden de llegada, que pagina logica
 * ocupa cada marco fisico. Se usa para saber cual reemplazar primero. */
int cola_marcos[NUM_MARCOS];   /* cola_marcos[m] = pagina logica en el marco m, o -1 si el marco esta libre */
int marcos_ocupados = 0;       /* cuantos marcos estan actualmente ocupados */
int siguiente_marco_a_reemplazar = 0; /* indice circular FIFO: proximo marco a liberar cuando todos esten llenos */

/* Contadores para estadisticas */
int contador_fallos = 0;
int contador_aciertos = 0;

/* ----------------------------------------------------------------------
 * inicializar_tabla_paginas
 * Marca todas las paginas como NO CARGADAS (-1) y todos los marcos
 * como LIBRES (-1). YA ESTA COMPLETA.
 * ---------------------------------------------------------------------- */
void inicializar_tabla_paginas(void) {
    int i;
    for (i = 0; i < NUM_PAGINAS; i++) {
        tabla_paginas[i] = -1;
    }
    for (i = 0; i < NUM_MARCOS; i++) {
        cola_marcos[i] = -1;
    }
    marcos_ocupados = 0;
    siguiente_marco_a_reemplazar = 0;
    contador_fallos = 0;
    contador_aciertos = 0;
}

/* ----------------------------------------------------------------------
 * mostrar_tabla_paginas
 * Imprime el estado actual de la tabla de paginas y de los marcos.
 * YA ESTA COMPLETA.
 * ---------------------------------------------------------------------- */
void mostrar_tabla_paginas(void) {
    int i;
    printf("\n--- Tabla de paginas (proceso) ---\n");
    for (i = 0; i < NUM_PAGINAS; i++) {
        if (tabla_paginas[i] == -1) {
            printf(" Pagina %d -> no cargada\n", i);
        } else {
            printf(" Pagina %d -> Marco %d\n", i, tabla_paginas[i]);
        }
    }
    printf("--- Marcos fisicos ---\n");
    for (i = 0; i < NUM_MARCOS; i++) {
        if (cola_marcos[i] == -1) {
            printf(" Marco %d -> libre\n", i);
        } else {
            printf(" Marco %d -> Pagina %d\n", i, cola_marcos[i]);
        }
    }
    printf("Fallos de pagina: %d | Aciertos: %d\n", contador_fallos, contador_aciertos);
}

/* ----------------------------------------------------------------------
 * cargar_pagina_en_memoria
 * Se invoca cuando ocurre un FALLO DE PAGINA para la pagina logica
 * "pagina". Debe conseguir un marco fisico donde cargarla:
 *
 *   - Si hay marcos libres (marcos_ocupados < NUM_MARCOS), use el
 *     primer marco libre disponible.
 *   - Si NO hay marcos libres, aplique reemplazo FIFO: elija el marco
 *     indicado por "siguiente_marco_a_reemplazar", quite de la tabla de
 *     paginas la pagina que estaba ahi (marquela como no cargada, -1),
 *     y avance siguiente_marco_a_reemplazar de forma circular
 *     ( (siguiente_marco_a_reemplazar + 1) % NUM_MARCOS ).
 *
 * En cualquier caso, al final debe:
 *   - Registrar en cola_marcos[marco_elegido] la nueva pagina logica.
 *   - Registrar en tabla_paginas[pagina] el marco_elegido.
 *   - Retornar el marco_elegido.
 *
 * TODO: Implemente esta funcion.
 * ---------------------------------------------------------------------- */
int cargar_pagina_en_memoria(int pagina) {
    /* TODO:
     * 1. Declare "int marco_elegido".
     * 2. Si (marcos_ocupados < NUM_MARCOS):
     *      - marco_elegido = marcos_ocupados  (siguiente posicion libre)
     *      - marcos_ocupados++
     *    Si no (todos los marcos estan llenos, hay que reemplazar):
     *      - marco_elegido = siguiente_marco_a_reemplazar
     *      - Busque en tabla_paginas cual pagina tenia asignado ese
     *        marco (cola_marcos[marco_elegido]) y marquela como -1 en
     *        tabla_paginas.
     *      - Actualice siguiente_marco_a_reemplazar =
     *          (siguiente_marco_a_reemplazar + 1) % NUM_MARCOS
     * 3. cola_marcos[marco_elegido] = pagina;
     * 4. tabla_paginas[pagina] = marco_elegido;
     * 5. Retorne marco_elegido.
     */

    return -1; /* Reemplace este retorno una vez implementada la logica */
}

/* ----------------------------------------------------------------------
 * traducir_direccion
 * Recibe una direccion logica y debe:
 *   1. Calcular el numero de pagina y el desplazamiento.
 *   2. Consultar la tabla de paginas:
 *      - Si la pagina YA esta cargada (tabla_paginas[pagina] != -1),
 *        es un ACIERTO (hit): incremente contador_aciertos.
 *      - Si NO esta cargada, es un FALLO DE PAGINA: incremente
 *        contador_fallos y llame a cargar_pagina_en_memoria(pagina)
 *        para traerla a memoria.
 *   3. Calcular la direccion fisica como:
 *        direccion_fisica = marco * TAM_PAGINA + desplazamiento
 *   4. Retornar la direccion fisica calculada.
 *
 * Debe tambien imprimir en pantalla el detalle de la traduccion
 * (pagina, desplazamiento, si fue acierto o fallo, marco asignado y
 * direccion fisica resultante).
 *
 * TODO: Implemente esta funcion.
 * ---------------------------------------------------------------------- */
int traducir_direccion(int direccion_logica) {
    /* TODO: calcule numero_pagina y desplazamiento */
    int numero_pagina = 0;   /* <-- calcule correctamente */
    int desplazamiento = 0;  /* <-- calcule correctamente */
    int marco;
    int direccion_fisica;

    /* TODO: valide que numero_pagina este dentro del rango [0, NUM_PAGINAS-1].
     * Si no lo esta, imprima un mensaje de "direccion invalida" y retorne -1. */

    /* TODO:
     * Si tabla_paginas[numero_pagina] != -1 -> ACIERTO:
     *     marco = tabla_paginas[numero_pagina];
     *     contador_aciertos++;
     *     imprima que fue un acierto.
     * Si no -> FALLO DE PAGINA:
     *     contador_fallos++;
     *     imprima que fue un fallo de pagina;
     *     marco = cargar_pagina_en_memoria(numero_pagina);
     */
    marco = 0; /* Reemplace por la logica anterior */

    direccion_fisica = 0; /* TODO: calcule marco * TAM_PAGINA + desplazamiento */

    printf("Direccion logica %d -> pagina %d, desplazamiento %d -> "
           "marco %d -> direccion fisica %d\n",
           direccion_logica, numero_pagina, desplazamiento, marco, direccion_fisica);

    return direccion_fisica;
}

/* ----------------------------------------------------------------------
 * main
 * YA ESTA COMPLETO. Simula una secuencia de accesos a direcciones
 * logicas generadas por un programa, y muestra el resultado de cada
 * traduccion junto con el estado final de la tabla de paginas.
 * ---------------------------------------------------------------------- */
int main() {
    /* Secuencia de direcciones logicas que "el proceso" va accediendo.
     * Con TAM_PAGINA = 256, la direccion 300 corresponde a la pagina 1
     * (300 / 256 = 1) con desplazamiento 44 (300 % 256 = 44), etc. */
    int secuencia[] = {0, 300, 600, 900, 1200, 1500, 50, 1800, 610, 2000};
    int total = sizeof(secuencia) / sizeof(secuencia[0]);
    int i, opcion, direccion;

    inicializar_tabla_paginas();

    printf("=============================================================\n");
    printf(" SIMULADOR DE MEMORIA VIRTUAL - PAGINACION CON REEMPLAZO FIFO\n");
    printf(" Tamano de pagina: %d bytes | Paginas logicas: %d | Marcos: %d\n",
           TAM_PAGINA, NUM_PAGINAS, NUM_MARCOS);
    printf("=============================================================\n");

    do {
        printf("\n1. Ejecutar secuencia de accesos de ejemplo\n");
        printf("2. Traducir una direccion logica manualmente\n");
        printf("3. Ver tabla de paginas y marcos\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                for (i = 0; i < total; i++) {
                    traducir_direccion(secuencia[i]);
                }
                mostrar_tabla_paginas();
                break;
            case 2:
                printf("Ingrese direccion logica (0 - %d): ", TAM_PAGINA * NUM_PAGINAS - 1);
                scanf("%d", &direccion);
                traducir_direccion(direccion);
                break;
            case 3:
                mostrar_tabla_paginas();
                break;
            case 0:
                printf("Saliendo del simulador...\n");
                break;
            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 0);

    return 0;
}
