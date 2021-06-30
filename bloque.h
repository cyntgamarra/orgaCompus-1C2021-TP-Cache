#ifndef ORGACOMPUS_1C2021_TP_CACHE_BLOQUE_H
#define ORGACOMPUS_1C2021_TP_CACHE_BLOQUE_H

typedef struct Bloque {
    unsigned int tag;
    char *data;
    unsigned int orden;    // para saber cual es el primero que entro (FIFO)
} bloque_t;

int bloque_init(bloque_t *self, size_t block_size, unsigned int tag, char *data, unsigned int orden);

int bloque_uninit(bloque_t *self);

int bloque_get_tag(bloque_t *self);

int bloque_get_data(bloque_t *self, char *buffer, size_t len);

#endif //ORGACOMPUS_1C2021_TP_CACHE_BLOQUE_H
