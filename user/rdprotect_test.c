#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    printf("\n===== INICIO TEST MRDPROTECT =====\n");

    // Obtener la dirección actual del heap
    char *addr = sbrk(0);
    printf("[1] Dirección inicial del heap: %p\n", addr);

    // Reservar una página completa
    sbrk(4096);
    printf("[2] Página reservada con sbrk(4096)\n");

    // Escritura inicial
    addr[0] = 'Z';
    printf("[3] Escritura inicial OK (addr[0] = 'Z')\n");

    // Aplicar protección contra lectura
    printf("[4] Aplicando mrdprotect(addr, 1)...\n");
    if (mrdprotect(addr, 1) < 0) {
        printf("ERROR: mrdprotect falló\n");
        exit(1);
    }
    printf("[4] mrdprotect aplicado correctamente\n");

    // Verificar que la escritura siga funcionando
    addr[0] = 'A';
    printf("[5] Escritura posterior permitida (addr[0] = 'A')\n");

    // Intento de lectura — aquí debe ocurrir el page fault
    printf("[6] Intentando lectura (DEBERÍA FALLAR)...\n");

    char c = addr[0];   // ← PAGE FAULT aquí
    printf("ERROR: la lectura NO falló, valor = %c\n", c);

    // Restaurar el permiso de lectura (solo se ejecuta si no hubo crash)
    printf("[7] Restaurando permisos con munrdprotect...\n");
    if (munrdprotect(addr, 1) < 0) {
        printf("ERROR: munrdprotect falló\n");
        exit(1);
    }

    printf("[8] Protección revertida correctamente.\n");

    printf("===== TEST FINALIZADO (esto no debería verse si el test es correcto) =====\n");
    exit(0);
}
