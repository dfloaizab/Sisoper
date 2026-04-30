# Sistemas Operativos 2026A, Universidad Santiago de Cali

## Ejercicio: Manipulación de Archivos en ANSI C en el Contexto de Sistemas Operativos

## Contexto y fundamentos

En los sistemas operativos modernos, los archivos constituyen la unidad
fundamental de almacenamiento persistente. A través del sistema de
archivos, el sistema operativo abstrae el acceso a dispositivos de
almacenamiento, permitiendo a los programas leer, escribir y organizar
información de manera estructurada.

En el lenguaje C, el manejo de archivos puede realizarse mediante dos
enfoques principales: las funciones de alto nivel definidas en la
biblioteca estándar (`stdio.h`) y las llamadas al sistema disponibles en
entornos POSIX (`open`, `read`, `write`, `close`, entre otras). En este
ejercicio se trabajará con funciones de alto nivel, sin perder de vista
su relación conceptual con el sistema operativo subyacente.

## Objetivo del ejercicio

El estudiante deberá completar un programa en ANSI C que realice
operaciones básicas sobre archivos de texto. El programa deberá abrir un
archivo de entrada, procesar su contenido y generar un archivo de salida
con información transformada.

Se busca que el estudiante aplique los siguientes conceptos:

Apertura de archivos en distintos modos, lectura secuencial de datos,
escritura en archivos de salida, manejo de errores en operaciones de
entrada/salida, uso de punteros a archivos (`FILE *`) y manipulación de
cadenas de caracteres.

## Enunciado

Se desea implementar un programa que reciba como entrada un archivo de
texto llamado `entrada.txt`, el cual contiene múltiples líneas de texto.
El programa deberá generar un archivo llamado `salida.txt` que contenga
el número total de líneas del archivo original, el número total de
palabras, y una copia del contenido original en la que todas las letras
minúsculas se conviertan a mayúsculas.

El estudiante deberá completar el código base proporcionado,
implementando las funciones necesarias para abrir correctamente los
archivos, leer línea por línea el archivo de entrada, contar palabras y
líneas, transformar caracteres, escribir los resultados en el archivo de
salida y manejar posibles errores.

## Código base (completar)

``` c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINEA 256

int contar_palabras(const char *linea);
void convertir_mayusculas(char *linea);

int main() {
    FILE *archivo_entrada;
    FILE *archivo_salida;

    char buffer[MAX_LINEA];
    int total_lineas = 0;
    int total_palabras = 0;

    /* TODO: Abrir archivo de entrada en modo lectura */

    /* TODO: Verificar si el archivo se abrió correctamente */

    /* TODO: Abrir archivo de salida en modo escritura */

    /* TODO: Verificar si el archivo de salida se abrió correctamente */

    while (fgets(buffer, MAX_LINEA, archivo_entrada) != NULL) {
        total_lineas++;

        /* TODO: Contar palabras en la línea y acumular */

        /* TODO: Convertir la línea a mayúsculas */

        /* TODO: Escribir la línea transformada en el archivo de salida */
    }

    /* TODO: Escribir estadísticas en el archivo de salida */

    /* TODO: Cerrar ambos archivos */

    printf("Proceso completado.\n");

    return 0;
}

int contar_palabras(const char *linea) {
    int contador = 0;
    int en_palabra = 0;

    /* TODO: Implementar lógica */

    return contador;
}

void convertir_mayusculas(char *linea) {
    int i = 0;

    /* TODO: Implementar conversión */

}
```

## Requisitos adicionales

El programa debe ser robusto ante errores. Si el archivo de entrada no
existe, el programa debe mostrar un mensaje adecuado y finalizar
correctamente. También debe cerrar todos los archivos abiertos.

Se recomienda probar con archivos vacíos y con contenido variado.

## Criterios de evaluación

Se evaluará la correcta implementación, el manejo adecuado de archivos,
la claridad del código y la aplicación de los conceptos fundamentales de
sistemas de archivos.
