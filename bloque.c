#include <malloc.h>
#include <string.h>
#include "bloque.h"

int bloque_init(bloque_t *self, size_t block_size, unsigned int tag, char *data, unsigned int orden) {
    self->tag = tag;

    self->data = malloc(sizeof(char) * block_size);
    memset(self->data, '\0', sizeof(self->data));
    strncpy(self->data, data, block_size);

    self->orden = orden;
    return 0;
}

int bloque_uninit(bloque_t *self) {
    free(self->data);
    return 0;
}

int bloque_get_tag(bloque_t *self) {
    return self->tag;
}

int bloque_get_data(bloque_t *self, char *buffer, size_t len) {
    memset(buffer, '\0', sizeof(len));
    strncpy(buffer, self->data, sizeof(self->data));
    return 0;
}

