/* matmul_parallel.c
   Compilar: gcc matmul_parallel.c -o matmul_parallel -lpthread
   Ejecución ejemplo: ./matmul_parallel 512 4
   (primer argumento: tamaño n, segundo: num hilos)
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//esta estructura guarda la información que se guarda a cada hilo que se va a ejecutar en paralelo:
typedef struct {
    int tid;
    int n;
    int start_row;
    int end_row; // [start_row, end_row)
    double *A; // n x n row-major
    double *B;
    double *C;
} MatMulArg;

//esta es la función que cada hilo ejecuta en paralelo (multiplica un rango de filas por un rango de columnas y guarda un resultado parcial):
void* matmul_worker(void* arg) {
    MatMulArg* a = (MatMulArg*)arg;
    int n = a->n;
    double *A = a->A, *B = a->B, *C = a->C;
    for (int i = a->start_row; i < a->end_row; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            // A[i][k] -> A[i*n + k], B[k][j] -> B[k*n + j]
            for (int k = 0; k < n; ++k) {
                sum += A[i*n + k] * B[k*n + j];
            }
            C[i*n + j] = sum;
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Uso: %s <n> <num_threads>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    if (n <= 0 || num_threads <= 0) { printf("Argumentos inválidos\n"); return 1; }

    // Reservar matrices
    double *A = malloc(sizeof(double) * n * n);
    double *B = malloc(sizeof(double) * n * n);
    double *C = malloc(sizeof(double) * n * n);
    if (!A || !B || !C) { perror("malloc"); return 1; }

    // Inicializar A y B con valores (ejemplo aleatorio)
    srand((unsigned)time(NULL));
    for (int i = 0; i < n*n; ++i) {
        A[i] = (double)(rand() % 100) / 10.0;
        B[i] = (double)(rand() % 100) / 10.0;
        C[i] = 0.0;
    }

    pthread_t threads[num_threads];
    MatMulArg args[num_threads];

    //ESTOS PARÁMETROS CORRESPONDEN A LA MULTIPLICACIÓN DE MATRICES:
    //argumentos que corresponden a la multiplicación de matrices: 
    //número de filas que le tocan a cada hilo
    int base = n / num_threads;
    //residuo para asignar una fila adicional a los hilos restantes   
    int rem = n % num_threads;
    //indice inicial de la fila a asignar
    int row = 0;

    // EJECUCIÓN DE LOS HILOS EN PARALELO
    // Crear hilos, asignar bloques de filas
    for (int t = 0; t < num_threads; ++t) {
        int chunk = base + (t < rem ? 1 : 0);
        args[t].tid = t;
        args[t].n = n;
        args[t].start_row = row;
        args[t].end_row = row + chunk;
        args[t].A = A;
        args[t].B = B;
        args[t].C = C;

        if (pthread_create(&threads[t], NULL, matmul_worker, &args[t]) != 0) {
            perror("pthread_create");
            return 1;
        }
        row += chunk;
    }

    // Esperar hilos
    for (int t = 0; t < num_threads; ++t) {
        pthread_join(threads[t], NULL);
    }

    // (Opcional) imprimir una celda para verificar
    printf("C[0][0] = %f\n", C[0]);

    free(A); free(B); free(C);
    return 0;
}
