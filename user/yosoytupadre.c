#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("PID=%d, PPID=%d\n", getpid(), getppid());
  exit(0);
}
