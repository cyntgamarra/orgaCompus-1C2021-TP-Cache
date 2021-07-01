#ifndef ORGACOMPUS_1C2021_TP_CACHE_VIA_H
#define ORGACOMPUS_1C2021_TP_CACHE_VIA_H

#include "bloque.h"

typedef struct via {
    unsigned int capacidad;
    unsigned int ocupados;
    int orden;
    unsigned int block_size;
    bloque_t **bloques;
} via_t;

int via_init(via_t *self, unsigned int capacidad, unsigned int block_size);

int via_uninit(via_t *self);

int via_agregar_bloque(via_t *self, unsigned int tag, char *data);

#endif //ORGACOMPUS_1C2021_TP_CACHE_VIA_H
