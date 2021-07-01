#include <malloc.h>
#include <string.h>
#include "via.h"

int via_init(via_t *self, unsigned int capacidad, unsigned int block_size) {
    self->capacidad = capacidad;
    self->ocupados = 0;
    self->orden = 0;
    self->block_size = block_size;
    self->bloques = malloc(sizeof(bloque_t) * capacidad);
    for (int i = 0; i < self->capacidad; i++) {
        self->bloques[i] = (bloque_t*)malloc(sizeof(bloque_t));
        bloque_init(self->bloques[i], self->block_size);
    }
}

int via_uninit(via_t *self) {
    for (int i = 0; i < self->capacidad; i++) {
        bloque_uninit(self->bloques[i]);
        free(self->bloques[i]);
    }
    free(self->bloques);
    return 0;
}

int via_agregar_bloque(via_t *self, unsigned int tag, char *data) {
    if (self->ocupados != self->capacidad) {
        self->bloques[self->ocupados]->tag = tag;
        bloque_modificar_data(self->bloques[self->ocupados], data, 5);
        self->bloques[self->ocupados]->orden = self->ocupados;
        self->ocupados++;
    }
    return 0;
}
