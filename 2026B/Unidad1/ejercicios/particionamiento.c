/* ============================================================================
 * SISTEMAS OPERATIVOS
 * Ejercicio 1 (PLANTILLA): Particionamiento de memoria
 * Algoritmos de asignacion: First Fit, Best Fit, Worst Fit, Next Fit
 *
 * INSTRUCCIONES:
 * Complete las funciones marcadas con el comentario "TODO" siguiendo las
 * indicaciones dentro de cada una. NO modifique las firmas (nombre,
 * parametros, tipo de retorno) de las funciones.
 *
 * El programa simula un bloque de memoria dividido en particiones fijas
 * de distinto tamano. Cada particion puede estar libre u ocupada por un
 * proceso. El menu permite asignar procesos usando el algoritmo elegido,
 * liberar procesos y visualizar el mapa de memoria.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARTICIONES 10
#define LIBRE 0
#define OCUPADA 1

/* Representa una particion de memoria */
typedef struct {
    int id;            /* Identificador de la particion (0..n-1)          */
    int tamano;        /* Tamano en KB de la particion                    */
    int estado;        /* LIBRE u OCUPADA                                 */
    int id_proceso;    /* Identificador del proceso alojado (-1 si LIBRE) */
} Particion;

/* Puntero global usado por Next Fit para recordar la ultima posicion
 * donde se realizo una asignacion. Se inicializa en 0. */
int puntero_next_fit = 0;

/* ----------------------------------------------------------------------
 * inicializar_particiones
 * Carga el arreglo de particiones con tamanos de ejemplo y las marca
 * todas como LIBRES. Esta funcion YA ESTA COMPLETA, no es necesario
 * modificarla; sirve como referencia de como se recorre el arreglo.
 * ---------------------------------------------------------------------- */
void inicializar_particiones(Particion particiones[], int n) {
    int tamanos[MAX_PARTICIONES] = {100, 500, 200, 300, 600, 150, 400, 250, 350, 120};
    int i;
    for (i = 0; i < n; i++) {
        particiones[i].id = i;
        particiones[i].tamano = tamanos[i];
        particiones[i].estado = LIBRE;
        particiones[i].id_proceso = -1;
    }
}

/* ----------------------------------------------------------------------
 * mostrar_particiones
 * Imprime en pantalla el mapa de memoria: id, tamano, estado y proceso
 * asignado (si aplica). Esta funcion YA ESTA COMPLETA.
 * ---------------------------------------------------------------------- */
void mostrar_particiones(Particion particiones[], int n) {
    int i;
    printf("\n----------------------------------------------------------\n");
    printf(" ID | Tamano(KB) | Estado  | Proceso\n");
    printf("----------------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        printf(" %-3d| %-11d| %-8s| %s\n",
               particiones[i].id,
               particiones[i].tamano,
               particiones[i].estado == LIBRE ? "LIBRE" : "OCUPADA",
               particiones[i].estado == OCUPADA ? "" : "-");
        if (particiones[i].estado == OCUPADA) {
            printf("     -> Proceso P%d\n", particiones[i].id_proceso);
        }
    }
    printf("----------------------------------------------------------\n");
}

/* ----------------------------------------------------------------------
 * first_fit
 * Recorre las particiones DESDE EL INICIO y retorna el indice de la
 * PRIMERA particion LIBRE cuyo tamano sea mayor o igual a tam_proceso.
 * Si no encuentra ninguna, retorna -1.
 *
 * TODO: Implemente el algoritmo First Fit.
 * ---------------------------------------------------------------------- */
int first_fit(Particion particiones[], int n, int tam_proceso) {
    /* TODO:
     * 1. Recorra el arreglo de particiones de i = 0 hasta n-1.
     * 2. Si particiones[i].estado == LIBRE Y particiones[i].tamano >= tam_proceso,
     *    retorne i inmediatamente (esa es la respuesta de First Fit).
     * 3. Si termina el recorrido sin encontrar ninguna, retorne -1.
     */

    return -1; /* Reemplace este retorno una vez implementada la logica */
}

/* ----------------------------------------------------------------------
 * best_fit
 * Recorre TODAS las particiones LIBRES que puedan alojar al proceso y
 * retorna el indice de la que tenga el MENOR tamano posible (el hueco
 * que deja menos fragmentacion interna). Si no hay ninguna, retorna -1.
 *
 * TODO: Implemente el algoritmo Best Fit.
 * ---------------------------------------------------------------------- */
int best_fit(Particion particiones[], int n, int tam_proceso) {
    /* TODO:
     * 1. Declare una variable "mejor_indice" inicializada en -1 y
     *    "mejor_tamano" inicializada en un valor muy grande (por ejemplo,
     *    el tamano maximo posible o el primero que encuentre valido).
     * 2. Recorra todas las particiones.
     * 3. Si esta LIBRE y su tamano >= tam_proceso Y es menor que
     *    "mejor_tamano" encontrado hasta ahora, actualice mejor_indice
     *    y mejor_tamano.
     * 4. Al terminar el recorrido, retorne mejor_indice.
     */

    return -1; /* Reemplace este retorno una vez implementada la logica */
}

/* ----------------------------------------------------------------------
 * worst_fit
 * Recorre TODAS las particiones LIBRES que puedan alojar al proceso y
 * retorna el indice de la que tenga el MAYOR tamano (deja el hueco mas
 * grande posible, para aprovecharlo despues). Si no hay ninguna, -1.
 *
 * TODO: Implemente el algoritmo Worst Fit.
 * ---------------------------------------------------------------------- */
int worst_fit(Particion particiones[], int n, int tam_proceso) {
    /* TODO:
     * Analogo a best_fit, pero en vez de buscar el menor tamano valido,
     * busque el MAYOR tamano valido entre las particiones libres que
     * alcancen para el proceso.
     */

    return -1; /* Reemplace este retorno una vez implementada la logica */
}

/* ----------------------------------------------------------------------
 * next_fit
 * Variante de First Fit que NO empieza a buscar desde el inicio del
 * arreglo, sino desde la posicion donde quedo la ULTIMA asignacion
 * (variable global puntero_next_fit). Si llega al final del arreglo,
 * continua la busqueda desde el inicio (busqueda circular), hasta
 * completar una vuelta completa. Si no encuentra ninguna, retorna -1.
 *
 * TODO: Implemente el algoritmo Next Fit.
 * ---------------------------------------------------------------------- */
int next_fit(Particion particiones[], int n, int tam_proceso) {
    /* TODO:
     * 1. Recorra las particiones en orden circular comenzando en
     *    puntero_next_fit: use un contador de 0 a n-1 y calcule el
     *    indice real como (puntero_next_fit + contador) % n.
     * 2. Si la particion en ese indice esta LIBRE y su tamano >=
     *    tam_proceso, actualice puntero_next_fit al indice SIGUIENTE
     *    ( (indice + 1) % n ) y retorne ese indice.
     * 3. Si completa las n vueltas sin encontrar nada, retorne -1.
     */

    return -1; /* Reemplace este retorno una vez implementada la logica */
}

/* ----------------------------------------------------------------------
 * asignar_particion
 * Marca la particion en la posicion "indice" como OCUPADA y le asigna
 * el id_proceso indicado. Esta funcion YA ESTA COMPLETA.
 * ---------------------------------------------------------------------- */
void asignar_particion(Particion particiones[], int indice, int id_proceso) {
    particiones[indice].estado = OCUPADA;
    particiones[indice].id_proceso = id_proceso;
}

/* ----------------------------------------------------------------------
 * liberar_particion
 * Busca la particion ocupada por el proceso "id_proceso" y la marca
 * como LIBRE nuevamente (simula la finalizacion de un proceso).
 * Retorna 1 si la libero exitosamente, 0 si no encontro el proceso.
 *
 * TODO: Implemente esta funcion.
 * ---------------------------------------------------------------------- */
int liberar_particion(Particion particiones[], int n, int id_proceso) {
    /* TODO:
     * 1. Recorra el arreglo buscando una particion OCUPADA cuyo
     *    id_proceso coincida con el parametro recibido.
     * 2. Si la encuentra, cambie su estado a LIBRE y su id_proceso a -1,
     *    y retorne 1.
     * 3. Si no la encuentra tras recorrer todo el arreglo, retorne 0.
     */

    return 0; /* Reemplace este retorno una vez implementada la logica */
}

/* ----------------------------------------------------------------------
 * main
 * Menu interactivo. YA ESTA COMPLETO: no es necesario modificarlo, pero
 * puede leerlo para entender como se usan las funciones anteriores.
 * ---------------------------------------------------------------------- */
int main() {
    Particion particiones[MAX_PARTICIONES];
    int n = MAX_PARTICIONES;
    int opcion, algoritmo, tam_proceso, id_proceso, indice;
    static int contador_procesos = 1;

    inicializar_particiones(particiones, n);

    do {
        printf("\n============ SIMULADOR DE PARTICIONAMIENTO DE MEMORIA ============\n");
        printf("1. Ver mapa de memoria\n");
        printf("2. Asignar un proceso\n");
        printf("3. Liberar un proceso\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                mostrar_particiones(particiones, n);
                break;

            case 2:
                printf("Tamano del proceso (KB): ");
                scanf("%d", &tam_proceso);

                printf("Algoritmo a usar:\n");
                printf(" 1. First Fit\n 2. Best Fit\n 3. Worst Fit\n 4. Next Fit\n");
                printf("Opcion: ");
                scanf("%d", &algoritmo);

                switch (algoritmo) {
                    case 1: indice = first_fit(particiones, n, tam_proceso); break;
                    case 2: indice = best_fit(particiones, n, tam_proceso);  break;
                    case 3: indice = worst_fit(particiones, n, tam_proceso); break;
                    case 4: indice = next_fit(particiones, n, tam_proceso);  break;
                    default:
                        printf("Algoritmo invalido.\n");
                        indice = -1;
                }

                if (indice == -1) {
                    printf("No hay particion disponible para un proceso de %d KB.\n", tam_proceso);
                } else {
                    id_proceso = contador_procesos++;
                    asignar_particion(particiones, indice, id_proceso);
                    printf("Proceso P%d asignado a la particion %d (%d KB).\n",
                           id_proceso, indice, particiones[indice].tamano);
                }
                break;

            case 3:
                printf("Id del proceso a liberar: ");
                scanf("%d", &id_proceso);
                if (liberar_particion(particiones, n, id_proceso)) {
                    printf("Proceso P%d liberado correctamente.\n", id_proceso);
                } else {
                    printf("No se encontro el proceso P%d.\n", id_proceso);
                }
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
