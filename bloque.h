#ifndef ORGACOMPUS_1C2021_TP_CACHE_BLOQUE_H
#define ORGACOMPUS_1C2021_TP_CACHE_BLOQUE_H

typedef struct Bloque {
    unsigned int tag;
    char *data;
    unsigned int v; // dirty bit, clean = 0, dirty = 1
    unsigned int block_size;
    int orden;    // para saber cual es el primero que entro (FIFO)
} bloque_t;

int bloque_init(bloque_t *self, size_t block_size);

int bloque_uninit(bloque_t *self);

int bloque_obtener_tag(bloque_t *self);

int bloque_modificar_data(bloque_t *self, char* data, size_t len);

int bloque_obtener_data(bloque_t *self, char *buffer, size_t len);

#endif //ORGACOMPUS_1C2021_TP_CACHE_BLOQUE_H
