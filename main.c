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

typedef unsigned char byte;

// Cada bloque tiene su tag, sus bits de dirty y si es valido o no. 
// Además tiene el último acceso y 128 bytes bajo el array bytes. 
typedef struct Bloque {
	unsigned int es_valido:1;	
	unsigned int es_dirty:1;		
	char ultimo_acceso;
	unsigned int tag:TAG;		
	byte bytes[TAMANIO_BLOQUE];
} Bloque;


// Esto se repite para cada vía.
// Bloques vía 1, vía 2, vía 3 y vía 4
typedef struct Set {
	Bloque bloques[VIAS];
} Set;


//Finalmente tenemos la caché que tiene 8 bloques por vía. 
typedef struct Cache {
	int cantidad_accesos_memoria;
	float cantidad_misses;
	Set sets[CANTIDAD_BLOQUES_POR_VIA];
} Cache;


typedef struct Memoria {
	Bloque bloques[TAMANIO_MEMORIA_PRINCIPAL];
} Memoria;

Cache cache;
Memory memoria;

//La función get miss rate() debe devolver el porcentaje de misses
//desde que se inicializó la caché.
//printf("Miss rate: %f.\n", get_miss_rate());
double get_miss_rate() {
	if (cache.cantidad_accesos_memoria != 0) {
		return cache.cantidad_misses / cache.cantidad_accesos_memoria;
	} else {
		return 0;
	}
}

//La función write_byte(unsigned int address, unsigned char value) 
//debe escribir el valor value en la posición address de memo-
//ria, y en la posición correcta del bloque que corresponde a address,
//si el bloque se encuentra en la caché. Si no se encuentra, debe escribir
//el valor solamente en la memoria.
unsigned int write_byte(unsigned int address, unsigned char value){
	cache.cantidad_accesos_memoria ++; // Voy a acceder a memoria, incremento contador

	unsigned int tag = get_tag(address);
	unsigned int offset = get_offset(address);

	// Busco en todas las vías
	for (int i = 0; i < VIAS; i++) {
		// Si el bloque se encuentra y es válido
		Bloque set_encontrado = find_set(address).bloques[i];
		if (tag == set_encontrado.tag && set_encontrado.es_valido) {
			//Le asigno el valor
			set_encontrado.bloques[i].bytes[offset] = value;
			//Seteamos validez y ocupación
			set_encontrado.bloques[i].es_dirty = 1;
			set_encontrado.bloques[i].es_valido = 1;
			//Ponemos la fecha del último acceso
			//set_encontrado.bloques[i].ultimo_acceso = ;
			return value;
		}
	}
}

//La función read byte(unsigned int address) debe buscar el valor
//del byte correspondiente a la posición address en la caché; si éste no
//se encuentra en la caché debe cargar ese bloque. El valor de retorno
//siempre debe ser el valor del byte almacenado en la dirección indicada.
unsigned int read_byte (unsigned int address){
	cache.cantidad_accesos_memoria ++;
	
	unsigned int tag = get_tag(address);
	unsigned int offset = get_offset(address);

	for (int i = 0; i < VIAS; i++) {
		Bloque bloque_encontrado = find_set(address).bloques[i];
		// Si el bloque se encuentra y es válido
		if (tag == bloque_encontrado.tag && bloque_encontrado.es_valido) {
			return bloque_encontrado.bytes[offset];
		}
	}	

	//No lo encontró, es un miss
	cache.cantidad_misses++;

	//Buscar bloque en memoria principal. 
	//read_tocache()

	// Buscar nuevamente con el nuevo bloque
	for (int i = 0; i < VIAS; i++) {
		Bloque bloque_encontrado = find_set(address).bloques[i];
		// Si el bloque se encuentra y es válido
		if (tag == bloque_encontrado.tag && bloque_encontrado.es_valido) {
			return bloque_encontrado.bytes[offset];
		}
	}	

	//¿Qué pasa si no lo encuentra? 
	//return??? 
}



//La función read tocache(unsigned int blocknum, unsigned int way, unsigned int set) 
//debe leer el bloque blocknum de memoria
//y guardarlo en el conjunto y vı́a indicados en la memoria caché.
void read_tocache(unsigned int blocknum, unsigned int way, unsigned int set) {
 

}


//La función compare tag(unsigned int tag, unsigned int set)
//debe devolver la vı́a en la que se encuentra almacenado el bloque 
//correspondiente a tag en el conjunto index, o -1 si ninguno de los tags
//coincide.
unsigned int compare_tag(unsigned int tag, unsigned int set){
	int i;
	for(i = 0; i < VIAS; i++) {
		if(set.bloques[i].tag == tag) {
			return i;
		}
	}
	return -1;
}

//La función select oldest() debe devolver la vı́a en la que está el
//bloque más “viejo” dentro de un conjunto, utilizando el campo corres-
//pondiente de los metadatos de los bloques del conjunto.
void select_oldest(){

}


//La función find set(unsigned int address) debe devolver el con-
//junto de caché al que mapea la dirección address.
//Busco en la vía por el indice de la dirección y devuelvo el bloque
//correspondiente.
Set find_set(int address) {
	return cache.sets[get_index(address)];
}

//La función get offset(unsigned int address) debe devolver el off-
//set del byte del bloque de memoria al que mapea la dirección address
unsigned int get_offset(unsigned int address) {
	int mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
	return (byte)(address & mask);
}

unsigned int get_index(unsigned int address) {
	address = address >> 7;
	int mask = (1 << 0) | (1 << 1) | (1 << 2);
	return (byte)(address & mask);
}

unsigned int get_tag(unsigned int address) {
	address = address >> 10;
	int mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
	return (byte)(address & mask);
}

void init_cache(){
	cache.cantidad_misses = 0;
	cache.cantidad_accesos_memoria = 0;
	int i;
	for (i = 0; i < CANTIDAD_BLOQUES_POR_VIA; i++) {
		Set set;
		int j;
		//Inicializo el bloque
		for(j = 0; j < VIAS; j++) {
			Bloque bloque;
			bloque.es_dirty = 0;
			bloque.es_valido = 0;
			bloque.ultimo_acceso = null;
			bloque.tag = -1;
			int k;
			//Inicializo los bytes que componen el bloque
			for(k = 0; k < TAMANIO_BLOQUE; k++) {
				bloque.bytes[k] = 0;
			}
			//Inicializo el set para las 4 vías
			set.bloques[j] = bloque;
		}
		//Inicializo toda la caché
		cache.sets[i] = set;
	}
}

void init_memoria_principal() {
	int i;
	//Inicializo los bloques de toda la memoria principal
	for (i = 0; i < TAMANIO_MEMORIA_PRINCIPAL; i++) {
		Bloque bloque;
		bloque.es_dirty = 0;
		bloque.es_valido = 0;
		bloque.ultimo_acceso = 0;
		bloque.tag = -1;
		int k;
		//Inicializo los bytes que componen el bloque
		for(k = 0; k < TAMANIO_BLOQUE; k++) {
			bloque.bytes[k] = 0;
		}
		// Inicializo toda la memoria principal
		memoria.bloques[i] = bloque;
	}
}

//La función init() debe inicializar la memoria principal simulada en
//0, los bloques de la caché como inválidos y la tasa de misses a 0.
void init() {
	init_cache(); 
	init_memoria_principal();
}

