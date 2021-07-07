#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>

#define ERROR_ARGUMENTO "El formato de los argumentos es incorrecto.\n"
#define ERROR_SIN_ARGUMENTOS "Debe ingresar algún argumento.\n"
#define ERROR_PROCESAR "Hubo un error al procesar el archivo.\n"
#define TAMANIO_MEMORIA_PRINCIPAL 65536 // 64 KB, en bytes

void show_help()
{
	printf("Usage:\n");
	printf("\t tp2 -h \n");
	printf("\t tp2 -V \n");
	printf("\t tp2 options archivo \n");
	printf("Options:\n");
	printf("\t -h, --help \t Imprime ayuda. \n");
	printf("\t -V, --version \t Versión del programa. \n");
	printf("\t -o, --output \t Archivo de salida. \n");
	printf("\t -w, --ways \t Cantidad de vı́as. \n");
	printf("\t -cs, --cachesize \t Tamaño del caché en kilobytes. \n");
	printf("\t -ws, --blocksize \t Tamaño de bloque en bytes. \n");
	printf("Examples:\n");
	printf("\t tp2 -w 4 -cs 8 -bs 16 prueba1.mem \n");
}

void show_version()
{
	printf("tp2 - \t Version 1.0 \n");
}

typedef struct bloque {
	unsigned int es_valido;   
	int orden;//orden FIFO de agregado
	int tag;	   
	unsigned char* bytes;
} bloque_t;

typedef struct via {
	bloque_t** bloques;
	int cant_bloques;
} via_t;

typedef struct cache_t {
	char cantidad_accesos_memoria;
	char cantidad_misses;
	via_t** vias;
	int cant_vias;
} cache_t;


typedef struct memoria_principal {
	unsigned char* datos;
} memoria_principal_t;

cache_t* cache;
memoria_principal_t* memoria;
FILE* salida;
int cant_vias;
int tam_cache;
int tam_bloque;
int cant_bloques;

bloque_t* crear_bloque(int tam_bloque) {
	bloque_t* nuevo = malloc(sizeof(bloque_t));
	nuevo->es_valido = 0;
	nuevo->orden = -1;
	nuevo->tag = -1; //tag invalido
	nuevo->bytes = malloc(sizeof(char)*tam_bloque);
	for (int i = 0; i < tam_bloque; i++) {
		nuevo->bytes[i] = 0;
	}
	return nuevo;
}

via_t* crear_via(int cant_bloques, int tam_bloque) {
	via_t* una_via = malloc(sizeof(via_t));
	una_via->bloques = malloc(sizeof(bloque_t*)*cant_bloques);
	for (int i = 0; i < cant_bloques; i++) {
		una_via->bloques[i] = crear_bloque(tam_bloque);
	}
	una_via->cant_bloques = cant_bloques;
	return una_via;
}

void crear_cache(int bloques, int cant_vias, int tam_bloque){
	cache = malloc(sizeof(cache_t));
	cache->cantidad_accesos_memoria = 0;
	cache->cantidad_misses = 0;
	cache->cant_vias = cant_vias;
	cache->vias = malloc(sizeof(via_t*)*cant_vias);
	for (int i = 0; i < cant_vias; i++) {
		cache->vias[i] = crear_via(cant_bloques, tam_bloque);
	}
}

void init() {
	cant_bloques = 1024 * tam_cache / cant_vias /  tam_bloque;
	crear_cache(cant_bloques, cant_vias, tam_bloque);
	memoria = malloc(sizeof(memoria));
	memoria->datos = malloc(sizeof(unsigned char)*TAMANIO_MEMORIA_PRINCIPAL);
	for (int i = 0; i < TAMANIO_MEMORIA_PRINCIPAL; i++) {
		memoria->datos[i] = 0;
	}
}

void destruir_estructuras() {
	free(memoria->datos);
	free(memoria);
	via_t** vias = cache->vias;
	for (int i = 0; i < cant_vias; i++) {
		for (int j = 0; j < vias[i]->cant_bloques; j++) {
			free(vias[i]->bloques[j]->bytes);
			free(vias[i]->bloques[j]);
		}
		free(vias[i]->bloques);
		free(vias[i]);
	}
	free(vias);
	free(cache);
}

void write_byte_tomem(int address, char value) {
	memoria->datos[address] = value;
}

char get_miss_rate() {
	if (cache->cantidad_accesos_memoria == 0) {
		return 0;
	}
	float mr = (float)cache->cantidad_misses / cache->cantidad_accesos_memoria;
	return mr * 100;
}

unsigned int find_set(int address) {
	return (address / tam_bloque) % cant_bloques;
}

unsigned int find_earliest(int setnum) {
	for (unsigned int i = 0; i < cache->cant_vias; i++) {
		if (cache->vias[i]->bloques[setnum]->orden == 0) {
			return i;
		}
	}
	return 0;
}

unsigned int get_offset(int address) {
	unsigned int offset_mask = tam_bloque - 1;
	return address & offset_mask;
}

unsigned int get_tag(int address) {
	return address / tam_bloque / cant_bloques;	
}

void read_block(int blocknum) {
	int address = blocknum * tam_bloque;
	int setnum = find_set(address);
	unsigned int tag = get_tag(address);
	int via_a_usar = cache->cant_vias;
	int vias_ocupadas = 0;
	
	for (int i = 0; i < cache->cant_vias; i++) {
		// busca bloque libre
		if (cache->vias[i]->bloques[setnum]->es_valido != 1) {
			via_a_usar = i;
			break;
		}
		//cuenta bloques ya ocupados
		if (cache->vias[i]->bloques[setnum]->es_valido == 1) {
			vias_ocupadas++;
		}
	}
	//busca via en orden FIFO
	if (via_a_usar == cache->cant_vias) {
		via_a_usar = find_earliest(setnum);
		for (int i = 0; i < cache->cant_vias; i++) {
			cache->vias[i]->bloques[setnum]->orden--;
		}
	}
	//copia contenido memoria
	for (int i = 0; i < tam_bloque; i++) {
		cache->vias[via_a_usar]->bloques[setnum]->bytes[i] = memoria->datos[address+i];
	}
	//asigna valores válidos al bloque
	cache->vias[via_a_usar]->bloques[setnum]->es_valido = 1;
	cache->vias[via_a_usar]->bloques[setnum]->tag = tag;
	cache->vias[via_a_usar]->bloques[setnum]->orden = vias_ocupadas;
}

char read_byte(int address, char *hit) {
	int setnum = find_set(address);
	unsigned int tag = get_tag(address);
	unsigned int offset = get_offset(address);

	*hit = 0;
	cache->cantidad_accesos_memoria++;
	for (int i = 0; i < cache->cant_vias; i++) {
		if (cache->vias[i]->bloques[setnum]->es_valido == 1
			&& cache->vias[i]->bloques[setnum]->tag == tag) {
			*hit = 1;
			return cache->vias[i]->bloques[setnum]->bytes[offset];
		}
	}
	cache->cantidad_misses += 1;
	read_block(address / tam_bloque);
	return memoria->datos[address];
}

char write_byte(int address, char value, char *hit) {
	int setnum = find_set(address);
	unsigned int tag = get_tag(address);
	unsigned int offset = get_offset(address);

	*hit = 0; //write through siempre es miss
	for (int i = 0; i < cache->cant_vias; i++) {
		if (cache->vias[i]->bloques[setnum]->es_valido == 1
			&& cache->vias[i]->bloques[setnum]->tag == tag) {
			cache->vias[i]->bloques[setnum]->bytes[offset] = value;
		}
	}
	write_byte_tomem(address, value);
	cache->cantidad_accesos_memoria++;
	cache->cantidad_misses += 1;
	return 0;
}

bool detecta_error() {
	char delimitadores[4] = "\n, ";
	char* parametro = strtok(NULL, delimitadores);
	if (parametro != NULL) {
		fprintf(stderr, ERROR_PROCESAR);
		return true;
	}
	return false;
}

bool hay_caracter_no_digito(char* cadena) {
	for (int i = 0; i < strlen(cadena); i++) {
		if (isdigit(cadena[i]) == 0) {
			return true;
		}
	}
	return false;
}

bool es_accion(char* cadena, char* accion) {
	return strcmp(cadena, accion) == 0;
}

bool posicion_valida(char* posicion, int limite) {
	if (posicion == NULL) {
		return false;
	}
	 return !hay_caracter_no_digito(posicion) && atoi(posicion) < limite;
}

bool ejecutar_accion(char* linea) {
	char* parametro_1;
	char linea_aux[10000];
	char delimitadores[4] = "\n, ";

	strcpy(linea_aux, linea);
	parametro_1 = strtok(linea_aux, delimitadores);

	if (es_accion(parametro_1, "MR")) {
		if (detecta_error()) {
			return false;
		}
		fprintf(salida, "Miss rate: %i%%\n", get_miss_rate());
		return true;
	}
	if (es_accion(parametro_1, "R")) {
		char* parametro_2 = strtok(NULL, delimitadores);
		if (!posicion_valida(parametro_2, TAMANIO_MEMORIA_PRINCIPAL)) {
			fprintf(stderr, ERROR_PROCESAR);
			return false;
		}
		if (detecta_error()) {
			return false;
		}
		char hit;
		fprintf(salida, "Valor leído: %d\n", (unsigned char)read_byte(atoi(parametro_2), &hit));
		if (hit) {
			fprintf(salida, "Hit\n");
		} else {
			fprintf(salida, "Miss\n");
		}
		return true;
	}
	if (es_accion(parametro_1, "W")) {
		char* parametro_2 = strtok(NULL, delimitadores);
		char* parametro_3 = strtok(NULL, delimitadores);
		if (!posicion_valida(parametro_2, TAMANIO_MEMORIA_PRINCIPAL) ||
			!posicion_valida(parametro_3, 256)) {
			fprintf(stderr, ERROR_PROCESAR);
			return false;
		}
		if (detecta_error()) {
			return false;
		}
		char hit;
		write_byte(atoi(parametro_2), (unsigned char)atoi(parametro_3), &hit);
		if (hit) {
			fprintf(salida, "Hit\n");
		} else {
			fprintf(salida, "Miss\n");
		}
		return true;
	}
	else {
		fprintf(stderr, ERROR_PROCESAR);
		return false;
	}
}

void procesar_archivo(char* archivo) {
	FILE* file = fopen(archivo, "r");
	if (!file) {
		fprintf(salida, ERROR_PROCESAR);
		return;
	}
	char* linea = NULL;
	size_t tam = 0;
	
	while (getline(&linea, &tam, file) != EOF) {
		fprintf(salida, "%s", linea);
		ejecutar_accion(linea);
		fprintf(salida, "\n");
	}
	free(linea);
	fclose(file);
}

int main(int argc, char** argv) {
	struct option args[] = {
			{"help",		no_argument,  		NULL,  'h'},
			{"version",		no_argument,  		NULL,  'V'},
			{"output",		required_argument,	NULL,  'o'},
			{"ways",		required_argument,	NULL,  'w'},
			{"cachesize",	required_argument,	NULL,  'c'},
			{"blocksize",	required_argument,	NULL,  'b'},
	};
	char arg_opt_cad[] = "hVo:w:c:b:";
	int arg_opcion;
	int arg_opt_idx = 0;
	char finalizar = false;
	char* nombre_salida = NULL;
	int requeridos = 0;
	bool hay_salida = false;
	
	if(argc == 1) {
		printf(ERROR_SIN_ARGUMENTOS);
		show_help();
		finalizar = true;
	}

	while((arg_opcion =
		getopt_long(argc, argv, arg_opt_cad, args, &arg_opt_idx)) != -1
					&& !finalizar) {
		switch(arg_opcion){
			case 'h':
				show_help();
				finalizar = true;
				break;
			case 'V':
				show_version();
				finalizar = true;
				break;
			case 'o':
				nombre_salida = strdup(optarg);
				hay_salida = true;
				break;
			case 'w':
				cant_vias = atoi(optarg);
				requeridos++;
				break;
			case 'c':
				tam_cache = atoi(optarg);
				requeridos++;
				break;
			case 'b':
				tam_bloque = atoi(optarg);
				requeridos++;
				break;
		}
	}
	
	if (requeridos < 3) {
		printf(ERROR_ARGUMENTO);
		finalizar = true;
	}
	char* entrada = strdup(argv[argc-1]);
	if(hay_salida) {
		salida = fopen(nombre_salida, "w");
		if (!salida) finalizar = true;
	} else {
		salida = stdout;
	}
	
	if(finalizar)
		return 0;

	init();
	procesar_archivo(entrada);

	free(entrada);
	free(nombre_salida);
	fclose(salida);
	destruir_estructuras();
	return 0;
}
