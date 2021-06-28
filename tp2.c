#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERROR -1
#define OK 0
#define ERROR_ARGUMENTO "El formato de los argumentos es incorrecto"
#define ERROR_SIN_ARGUMENTOS "Debe ingresar algún argumento"
#define ERROR_PROCESAR "Hubo un error al procesar el archivo"	

#define CANTIDAD_VIAS 3
#define TAMANIO_CACHE 5
#define TAMANIO_BLOQUE 7
#define ARCHIVO_SALIDA 8

#define TAMANIO_NOMBRE_ARCHIVO_MAXIMO 1000

void show_help();
void show_version();
void init();
unsigned int find_set(int address);
unsigned int find_earliest(int setnum);
void read_block(int blocknum);
void write_byte_tomem(int address, char value);
char read_byte(int address, char *hit);
char write_byte(int address, char value, char *hit);
char get_miss_rate();

int main(int argc, char const *argv[]) {
 
    char archivo_salida[TAMANIO_NOMBRE_ARCHIVO_MAXIMO];
 
    // Se chequea que el usuario ingrese algún argumento
    if (argc == 1){
        fprintf(stderr, "%s\n", ERROR_SIN_ARGUMENTOS);
        show_help();
        return ERROR;
    }	

    // Caso en el que se ingresa -h o -V
    // Se chequea que se haya ingresado para consultar el mensaje de ayuda o la versión.
    // Hay error sino.
    if (argc == 2){
        if (!strcmp(argv[1], "-h")){
            show_help();
            return OK;
            
        }else if (!strcmp(argv[1], "-V")){
            show_version();
            return OK;
        }else{
            fprintf(stderr, "%s\n", ERROR_ARGUMENTO);
            show_help();
            return ERROR;
        }
    }
    
    // Caso en el se ingresan todos los parámetros. Hay error sino
    if (argc == 8){
        // VALIDAR QUE LOS ARGUMENTOS SEAN VALIDOS
        int cantidad_vias = atoi(argv[CANTIDAD_VIAS]);
        int cache_size = atoi(argv[TAMANIO_CACHE]);
        int block_size = atoi(argv[TAMANIO_BLOQUE]);
        strcpy(archivo_salida, argv[ARCHIVO_INICIAL]);
        int procesar = procesar_archivo( cantidad_vias, cache_size, block_size, archivo_salida )
        if procesar{
           return OK;
        else{
            printf(stderr, "%s\n", ERROR_PROCESAR);
            return ERROR;
        }
        }
        	
    }
    // Si no se ingresaron todos los parametros hay error
    else{
         fprintf(stderr, "%s\n", ERROR_ARGUMENTO);
         show_help();
         return ERROR;
    }
    
    

    return OK;

}

void show_help()
{
    printf("Usage:\n");
    printf("\t tp2 -h \n");
    printf("\t tp2 -V \n");
    printf("\t tp2 options archivo \n");
    printf("Options:\n");
    printf("\t -h, --help \t Imprime ayuda. \n");
    printf("\t -V, --version \t Version del programa. \n");
    printf("\t -o, --output ,Archivo de salida. \n");
    printf("Examples:\n");
    printf("\t tp2 -w 4 -cs 8 -bs 16 prueba1.mem \n");

}

void show_version()
{
    printf(" tp2 - \t Version 1.0 \n");
}
