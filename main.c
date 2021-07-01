#include <stdio.h>
#include "bloque.h"
#include "via.h"

int main() {
    printf("Hello, World!\n");
    bloque_t bloque;
    via_t via;
    via_init(&via, 10, 10);
    char *data = "holis";
    via_agregar_bloque(&via, 555, data);
    via_uninit(&via);
    return 0;
}
