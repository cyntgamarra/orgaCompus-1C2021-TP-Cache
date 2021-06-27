#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Constantes

#define OK 0
#define ERROR -1
#define CANTIDAD_DE_ARGUMENTOS 2
#define ERROR_ARGUMENTO "La cantidad de argumentos ingresada es incorrecta"
#define ERROR_ARCHIVO_LECTURA "No se pudo abrir el archivo inicial correctamente"
#define ERROR_FORMATO_ARCHIVO_LECTURA "El formato del archivo inicial es incorrecto"
#define LEER "r"
#define SALTO_DE_LINEA '\n'
#define MAXIMO_DE_LINEA 100
#define LINEA_INVALIDA false
#define TODO_OK true
#define TAMANIO_BYTE 256

#define VIAS 4                  //4WSA cache
#define TAMANIO_BLOQUE 64      //bytes
#define CANTIDAD_BLOQUES_POR_VIA 16
#define TAMANIO_INSTRUCCION 16  //bits
#define TAMANIO_MEMORIA_PRINCIPAL 65536 //bytes; 64 KB

/* bits - direccion */
#define OFFSET 6
#define INDEX 3
#define TAG 7

#define TAMANIO_CACHE 64            //2^INDEX = 2^6


//                      ----- Diseño de Caché ----- //
//        // --- conjunto[0] --- // ...  // --- conjunto[3] --- //   
//        //=====================// ...  //=====================// 
// set[0] //      bloque 0       // ...  //      bloque 0       //
// set[1] //      bloque 1       // ...  //      bloque 1       //
//  ...   //        ...          // ...  //        ...          //
// set[7] //      bloque 7       // ...  //      bloque 7       //
//        //=====================// ...  //=====================// 



// Diseño de memoria principal //
//[0] bloques --- 128 bytes


// [511] 


typedef unsigned char byte;

// Cada bloque tiene su tag, sus bits de dirty y si es valido o no. 
// Además tiene el último acceso y 128 bytes bajo el array bytes. 
typedef struct Bloque {
    unsigned int es_valido:1;   
    int temp_ultimo_acceso; // @TODO Temporizador, chequear
    unsigned int tag:TAG;       
    byte bytes[TAMANIO_BLOQUE];
} Bloque;


// Esto se repite para cada vía.
// Bloques vía 1, vía 2, vía 3 y vía 4
typedef struct Set {
    Bloque conjunto[VIAS];
} Set;


//Finalmente tenemos la caché que tiene 8 sets por vía. 
typedef struct Cache {
    int cantidad_accesos_memoria;
    float cantidad_misses;
    Set sets[CANTIDAD_BLOQUES_POR_VIA];
} Cache;


typedef struct Memoria {
    unsigned char datos[TAMANIO_MEMORIA_PRINCIPAL];
} Memoria;

Cache cache;
Memoria memoria;

// Primitivas a implementar

void init();

unsigned int get_offset(unsigned int address);

unsigned int find_set(unsigned int address);

unsigned int select_oldest(unsigned int setnum);

int compare_tag(unsigned int tag, unsigned int set);

void read_tocache(unsigned int blocknum, unsigned int way, unsigned int set);

void write_tocache(unsigned int address, unsigned char value);

unsigned char read_byte(unsigned int address);

void write_byte(unsigned int address, unsigned char value);

float get_miss_rate();

void init_cache();

void init_memoria_principal();

// Primitivas adicionales


bool son_todos_digitos(char string[]);

bool ejecutar_accion(char linea[]);

int main(int argc, char const *argv[]) {

    FILE* archivo_lectura;
    char linea[MAXIMO_DE_LINEA];

    init();

    if(argc == CANTIDAD_DE_ARGUMENTOS) {
        archivo_lectura = fopen(argv[1], LEER);
        if(!archivo_lectura) {
            fprintf(stderr, "%s\n", ERROR_ARCHIVO_LECTURA);
            return ERROR;
        }

        fgets(linea, MAXIMO_DE_LINEA, archivo_lectura);
        while(!feof(archivo_lectura)) {
            printf("%s", linea);

            int todo_ok = ejecutar_accion(linea);

            if(!todo_ok) {
             //   fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
                return ERROR;
            }

            fgets(linea, MAXIMO_DE_LINEA, archivo_lectura);
        }
    }
    else {
        fprintf(stderr, "%s\n", ERROR_ARGUMENTO);
        return ERROR;
    }


    fclose(archivo_lectura);

    return OK;
}

bool ejecutar_accion(char linea[]) {

    char* parametro_1;
    char linea_aux[MAXIMO_DE_LINEA];
    char delimitadores[] = "\n, ";

    strcpy(linea_aux, linea);
    parametro_1 = strtok(linea_aux, delimitadores);

    if(!strcmp(parametro_1, "FLUSH")) { // Me fijo que lo primero sea FLUSH
        char* parametro_2 = strtok(NULL, delimitadores);
        if(parametro_2 == NULL) { // Me fijo que no haya nada despues de FLUSH
            init_cache();
            return TODO_OK;
        }
        else {
            fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
            return LINEA_INVALIDA;
        }
    }
    else if(!strcmp(parametro_1, "MR")) { // Me fijo que lo primero sea MR
        char* parametro_2 = strtok(NULL, delimitadores);
        if(parametro_2 == NULL) { // Me fijo que no haya nada despues de MR
            printf("Miss rate: %f%%\n", get_miss_rate());
              return TODO_OK;
        }
        else {
            fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
            return LINEA_INVALIDA;
        }
    }
    else if(!strcmp(parametro_1, "R")) { // Me fijo que lo primero sea R
        char* parametro_2 = strtok(NULL, delimitadores);
        if(parametro_2 != NULL && (atoi(parametro_2) < TAMANIO_MEMORIA_PRINCIPAL) && son_todos_digitos(parametro_2)) { // Me fijo que lo segundo sea 'ddddd'
            char* parametro_3 = strtok(NULL, delimitadores);
            if(parametro_3 == NULL){ // Me fijo que no haya nada despues                
                printf("Valor leído: %u\n\n", read_byte(atoi(parametro_2)));
                return TODO_OK;
            }
            else {
                fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
                return LINEA_INVALIDA;
            }
        }
        else {
            fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
            return LINEA_INVALIDA;
        }
    }
    else if(!strcmp(parametro_1, "W")) { // Me fijo que lo primero sea W
        char* parametro_2 = strtok(NULL, delimitadores);

        if(parametro_2 != NULL && (atoi(parametro_2) < TAMANIO_MEMORIA_PRINCIPAL) && son_todos_digitos(parametro_2)) { // Me fijo que lo segundo sea 'ddddd'
            char* parametro_3 = strtok(NULL, delimitadores);
            if(parametro_3 != NULL && (atoi(parametro_3) < TAMANIO_BYTE) && son_todos_digitos(parametro_3)){ // Me fijo que lo tercero sea 'vvv'
                char* parametro_4 = strtok(NULL, delimitadores);
                if(parametro_4 == NULL) { // Me fijo que no haya nada despues
                    write_byte(atoi(parametro_2), atoi(parametro_3));
                    printf("\n");
                    return TODO_OK;
                }
                else {
                    fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
                    return LINEA_INVALIDA;
                }
            }
            else {
                fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
                return LINEA_INVALIDA;
            }
        }
        else {
            fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
            return LINEA_INVALIDA;
        }
    }
    else {
        fprintf(stderr, "%s\n", ERROR_FORMATO_ARCHIVO_LECTURA);
        return LINEA_INVALIDA;
    }
}

bool son_todos_digitos(char string[]) {
    bool son_digitos = true;

    for(int i = 0; i < strlen(string); i++) {
        if(isdigit(string[i]) == 0) {
            son_digitos = false;
        }
    }

    return son_digitos;
}

//La función get offset(unsigned int address) debe devolver el off-
//set del byte del bloque de memoria al que mapea la dirección address
unsigned int get_offset(unsigned int address) {
    int bit_mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
    return (byte)(address & bit_mask);
}


unsigned int get_tag(unsigned int address) {
    address = address >> 10;
    int mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
    return (byte)(address & mask);
}

//La función get miss rate() debe devolver el porcentaje de misses
//desde que se inicializó la caché.
//printf("Miss rate: %f.\n", get_miss_rate());
float get_miss_rate() {
    if (cache.cantidad_accesos_memoria != 0) {
        float mr = cache.cantidad_misses / cache.cantidad_accesos_memoria;
        return mr * 100;
    } else {
        return 0;
    }
}

//La función find set(unsigned int address) debe devolver el con-
//junto de caché al que mapea la dirección address.
//Busco en la vía por el indice de la dirección y devuelvo el bloque
//correspondiente.
unsigned int find_set(unsigned int address) {
    return (address / TAMANIO_BLOQUE) % CANTIDAD_BLOQUES_POR_VIA;
}

//La función select oldest() debe devolver la vı́a en la que está el
//bloque más “viejo” dentro de un conjunto, utilizando el campo corres-
//pondiente de los metadatos de los bloques del conjunto.
unsigned int select_oldest(unsigned int setnum){
    unsigned int via_lru = 0;
    for(int j = 0; j < VIAS; j++) {
        if (cache.sets[setnum].conjunto[via_lru].temp_ultimo_acceso 
                            < cache.sets[setnum].conjunto[j].temp_ultimo_acceso) { //@TODO 
            via_lru = j;
        }
    }
    return via_lru; //@TODO verificar qué debería devolver
}

void incrementar_temp(unsigned int setnum){
    for(int j = 0; j < VIAS; j++) {
        if(cache.sets[setnum].conjunto[j].es_valido){
            cache.sets[setnum].conjunto[j].temp_ultimo_acceso++;
        }
    }
}


//La función write_byte(unsigned int address, unsigned char value) 
//debe escribir el valor value en la posición address de memo-
//ria, y en la posición correcta del bloque que corresponde a address,
//si el bloque se encuentra en la caché. Si no se encuentra, debe escribir
//el valor solamente en la memoria.
void write_byte(unsigned int address, unsigned char value){
    // Voy a acceder a memoria, incremento contador
    cache.cantidad_accesos_memoria ++; 

    unsigned int tag = get_tag(address);
    unsigned int setnum = find_set(address);
    
    unsigned int via = compare_tag(tag, setnum);

    // Escribo en memoria
    memoria.datos[address] = value;

    // Escribo en caché 
    if ((int)via >= 0){
        printf("Hit\n" );
        write_tocache(address,value);
        incrementar_temp(setnum);
    }else{
        printf("Miss\n" );
        cache.cantidad_misses++;
    }
}


unsigned int find_blocknum(unsigned int address){
    return (address / TAMANIO_BLOQUE);
}

//La función read byte(unsigned int address) debe buscar el valor
//del byte correspondiente a la posición address en la caché; si éste no
//se encuentra en la caché debe cargar ese bloque. El valor de retorno
//siempre debe ser el valor del byte almacenado en la dirección indicada.
unsigned char read_byte (unsigned int address){
    
    // Voy a acceder a memoria, incremento contador
    cache.cantidad_accesos_memoria ++; 
    
    unsigned int tag = get_tag(address);
    unsigned int offset = get_offset(address);

    unsigned int setnum = find_set(address);
    unsigned int via = compare_tag(tag, setnum);
    unsigned int blocknum = find_blocknum(address);

//  la vía no es válida, no se encuentra en cache o es inválido
    if ((int)via < 0 || !cache.sets[setnum].conjunto[via].es_valido) { 
        //No lo encontró, es un miss
        cache.cantidad_misses++;
        printf("Miss\n" );
        //Buscar bloque en memoria principal. 
        via = select_oldest(setnum);
        read_tocache(blocknum, via, setnum); //@TODO Pendiente 
    }else{
        printf("Hit\n");
    }
    incrementar_temp(setnum);
    return cache.sets[setnum].conjunto[via].bytes[offset];
    
}


//La función read tocache(unsigned int blocknum, unsigned int way, unsigned int set) 
//debe leer el bloque blocknum de memoria
//y guardarlo en el conjunto y vı́a indicados en la memoria caché.
void read_tocache(unsigned int blocknum, unsigned int via, unsigned int setnum) {
 
    for (int i=0; i < TAMANIO_BLOQUE; i++ ){
        cache.sets[setnum].conjunto[via].bytes[i] = memoria.datos[blocknum*TAMANIO_BLOQUE+i];
    }

    cache.sets[setnum].conjunto[via].es_valido = 1; 
    cache.sets[setnum].conjunto[via].temp_ultimo_acceso = 0;
    cache.sets[setnum].conjunto[via].tag = get_tag(blocknum*TAMANIO_BLOQUE);
}


void write_tocache(unsigned int address, unsigned char value){
    unsigned int tag = get_tag(address);
    unsigned int setnum = find_set(address);
    unsigned int offset = get_offset(address);
    int via = compare_tag(tag, setnum);
        
    if (tag == cache.sets[setnum].conjunto[via].tag 
        && cache.sets[setnum].conjunto[via].es_valido) {
        //Le asigno el valor
        cache.sets[setnum].conjunto[via].bytes[offset] = value;
        //Seteamos validez y ocupación
        cache.sets[setnum].conjunto[via].es_valido = 1;
    }
}


//La función compare tag(unsigned int tag, unsigned int set)
//debe devolver la vı́a en la que se encuentra almacenado el bloque 
//correspondiente a tag en el conjunto index, o -1 si ninguno de los tags
//coincide.
int compare_tag(unsigned int tag, unsigned int num_set){
    Set set = cache.sets[num_set];
    for(int i = 0; i < VIAS; i++) {
        if(set.conjunto[i].tag == tag) {
            return i;
        }
    }
    return -1;
}

void init_cache(){
    cache.cantidad_misses = 0;
    cache.cantidad_accesos_memoria = 0;
    for (int i = 0; i < CANTIDAD_BLOQUES_POR_VIA; i++) {
        Set set;
        //Inicializo el bloque
        for(int j = 0; j < VIAS; j++) {
            Bloque bloque;
            bloque.es_valido = 0;
            bloque.temp_ultimo_acceso = 0; //@TODO chequear cuando esté construido
            bloque.tag = -1;
            //Inicializo los bytes que componen el bloque
            for(int k = 0; k < TAMANIO_BLOQUE; k++) {
                bloque.bytes[k] = 0;
            }
            //Inicializo el set para las 4 vías
            set.conjunto[j] = bloque;
        }
        //Inicializo toda la caché
        cache.sets[i] = set;
    }
}

void init_memoria_principal() {
    for(int i = 0; i < TAMANIO_MEMORIA_PRINCIPAL; i++) {
        memoria.datos[i] = 0;
    }
}

//La función init() debe inicializar la memoria principal simulada en
//0, los bloques de la caché como inválidos y la tasa de misses a 0.
void init() {
    init_cache(); 
    init_memoria_principal();
}
