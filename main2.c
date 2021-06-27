#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define VIAS 4					//4WSA cache
#define TAMANIO_BLOQUE 128	    //bytes
#define CANTIDAD_BLOQUES_POR_VIA 8
#define TAMANIO_INSTRUCCION 16  //bits
#define TAMANIO_MEMORIA_PRINCIPAL 65536	//bytes; 64 KB

/* bits - direccion */
#define OFFSET 6
#define INDEX 3
#define TAG 7

#define TAMANIO_CACHE 64			//2^INDEX = 2^6

//                      ----- Diseño de Caché ----- //
//        // --- conjunto[0] --- // ...  // --- conjunto[3] --- //   
//        //=====================// ...  //=====================// 
// set[0] //      bloque 0       // ...  //      bloque 0       //
// set[1] //      bloque 1       // ...  //      bloque 1       //
//  ...   //        ...          // ...  //        ...          //
// set[7] //      bloque 7       // ...  //      bloque 7       //
//        //=====================// ...  //=====================// 

typedef unsigned char byte;

// Cada bloque tiene su tag, sus bits de dirty y si es valido o no. 
// Además tiene el último acceso y 128 bytes bajo el array bytes. 
typedef struct Bloque {
    unsigned int es_valido:1;   
    unsigned int es_dirty:1;        
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
		return cache.cantidad_misses / cache.cantidad_accesos_memoria;
	} else {
		return 0;
	}
}

//La función find set(unsigned int address) debe devolver el con-
//junto de caché al que mapea la dirección address.
//Busco en la vía por el indice de la dirección y devuelvo el bloque
//correspondiente.
unsigned int find_set(unsigned int address) {
    return address / TAMANIO_BLOQUE % CANTIDAD_BLOQUES_POR_VIA;
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
	
	int via = compare_tag(tag, setnum);

	// Escribo en memoria
	memoria.datos[address] = value;

	// Escribo en caché 
	if (via >= 0){
		write_tocache(address,value);
	}
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

	if (via < 0){ //la vía no es válida, no se encuentra en cache
		//No lo encontró, es un miss
		cache.cantidad_misses++;
		//Buscar bloque en memoria principal. 
		via = select_oldest(setnum);
		//read_tocache() @TODO Pendiente 
	}

	Bloque bloque_encontrado = cache.sets[setnum].conjunto[via];
	if (tag == bloque_encontrado.tag && bloque_encontrado.es_valido) {
		return bloque_encontrado.bytes[offset];
	}

	//@TODO ¿Qué pasa si no lo encuentra ni en memoria? 
	//return??? 
}



//La función read tocache(unsigned int blocknum, unsigned int way, unsigned int set) 
//debe leer el bloque blocknum de memoria
//y guardarlo en el conjunto y vı́a indicados en la memoria caché.
void read_tocache(unsigned int blocknum, unsigned int way, unsigned int set) {
 
// @TODO
}

void write_tocache(unsigned int address, unsigned char value){
 	unsigned int tag = get_tag(address);
	unsigned int setnum = find_set(address);
	unsigned int offset = get_offset(address);
	int via = compare_tag(tag, setnum);
 	
 	Bloque bloque_encontrado = cache.sets[setnum].conjunto[via];
	
	if (tag == bloque_encontrado.tag && bloque_encontrado.es_valido) {
		//Le asigno el valor
		bloque_encontrado.bytes[offset] = value;
		//Seteamos validez y ocupación
		bloque_encontrado.es_dirty = 1;
		bloque_encontrado.es_valido = 1;
		bloque_encontrado.temp_ultimo_acceso++; //@TODO chequear!
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
			bloque.es_dirty = 0;
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

