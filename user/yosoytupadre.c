// user/yosoytupadre.c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // PID del proceso actual y el de su padre directo
  int me  = getpid();
  int p1  = getppid();

  // Probar la syscall nueva para distintos niveles:
  //  anc(0): yo mismo
  //  anc(1): padre
  //  anc(2): abuelo
  //  anc(3): probablemente -1 (si ya no hay más arriba)
  int a0 = getancestor(0);
  int a1 = getancestor(1);
  int a2 = getancestor(2);
  int a3 = getancestor(3);

  printf("PID=%d, PPID=%d\n", me, p1);
  printf("anc(0)=%d, anc(1)=%d, anc(2)=%d, anc(3)=%d\n", a0, a1, a2, a3);

  exit(0);
}
