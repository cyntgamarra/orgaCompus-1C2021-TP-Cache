#include <malloc.h>
#include <string.h>
#include "bloque.h"

int bloque_init(bloque_t *self, size_t block_size) {
    self->data = (char*)malloc(sizeof(char) * block_size);
    self->block_size = block_size;
    self->v = 0;
    return 0;
}

int bloque_uninit(bloque_t *self) {
    free(self->data);
    return 0;
}

int bloque_modificar_data(bloque_t *self, char* data, size_t len) {
    memset(self->data, '\0', sizeof(self->block_size));
    strncpy(self->data, data, len);
    return 0;
}

int bloque_obtener_tag(bloque_t *self) {
    return self->tag;
}

int bloque_obtener_data(bloque_t *self, char *buffer, size_t len) {
    memset(buffer, '\0', sizeof(len));
    strncpy(buffer, self->data, sizeof(self->data));
    return 0;
}

