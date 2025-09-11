// user/yosoytupadre.c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // 1) Capturar datos base
  int me  = getpid();
  int p1  = getppid();

  // 2) Probar la syscall nueva para distintos niveles
  int a0 = getancestor(0); // yo mismo
  int a1 = getancestor(1); // padre
  int a2 = getancestor(2); // abuelo (tipicamente init=1)
  int a3 = getancestor(3); // debería ser -1 (no hay más arriba)

  // 3) Mostrar PIDs crudos
  printf("\n=== Prueba de getppid() y getancestor() ===\n");
  printf("PID actual (getpid)      : %d\n", me);
  printf("PID del padre (getppid)  : %d\n", p1);
  printf("Ancestros: anc(0)=%d, anc(1)=%d, anc(2)=%d, anc(3)=%d\n",
         a0, a1, a2, a3);

  // 4) Checks de consistencia visibles ([OK]/[FAIL])
  printf("\n=== Checks de consistencia ===\n");
  printf("%s anc(0) == getpid\n",   (a0 == me) ? "[OK] " : "[FAIL] ");
  printf("%s anc(1) == getppid\n",  (a1 == p1) ? "[OK] " : "[FAIL] ");
  printf("%s anc(3) == -1 (sin ancestro más arriba)\n",
         (a3 == -1) ? "[OK] " : "[FAIL] ");

  // 5) “Arbolito” ASCII parcial (según lo que exista)
  printf("\n=== Árbol de procesos (parcial) ===\n");
  if (a2 != -1) {
    // típico: 1 -> 2 -> me
    printf("%d (posible init) -> %d (posible sh) -> %d (yo)\n", a2, a1, a0);
  } else {
    // si no hay abuelo, mostramos lo que hay
    printf("%d (padre) -> %d (yo)\n", a1, a0);
  }

  exit(0);
}
