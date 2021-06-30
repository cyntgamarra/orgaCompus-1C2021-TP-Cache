#include <stdio.h>
#include "bloque.h"

int main() {
    printf("Hello, World!\n");
    bloque_t bloque;
    char *data = "holis";
    bloque_init(&bloque, 8, 200, data, 1);
    int tag = bloque_get_tag(&bloque);
    char buffer[20];
    bloque_get_data(&bloque, buffer, 20);
    printf(buffer);
    bloque_uninit(&bloque);
    return 0;
}
