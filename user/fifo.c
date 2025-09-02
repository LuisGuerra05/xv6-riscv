// user/fifo.c — FIFO demo con líneas "atómicas" (una write por línea)
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// --- helpers para formatear sin mezclar líneas ---
static int utoa(uint x, char *s) {
  char t[16]; int i=0; do { t[i++]='0'+(x%10); x/=10; } while(x);
  int n=i; while(i--) *s++=t[i]; return n;
}
static int append(char *d, int n, const char *s){ while(*s) d[n++]=*s++; return n; }
static int append_u(char *d, int n, uint x){ return n+utoa(x, d+n); }
static void puts1(const char *tag, const char *evt, int pid_override) {
  char buf[128]; int n=0;
  n = append(buf, n, "[ticks="); n = append_u(buf, n, (uint)uptime());
  n = append(buf, n, "] ");       n = append(buf, n, evt);
  n = append(buf, n, " ");        n = append(buf, n, tag);
  n = append(buf, n, " (pid=");
  int pid = (pid_override >= 0 ? pid_override : getpid());
  n = append_u(buf, n, (uint)pid);
  n = append(buf, n, ")\n");
  write(1, buf, n);
}

static void cpu_work(const char *tag) {
  puts1(tag, "START", -1);
  sleep(1);                 // trabajo mínimo, rápido y visible
  puts1(tag, "END", -1);
  exit(0);
}

int
main(int argc, char **argv)
{
  // Cabecera
  {
    char buf[64]; int n=0;
    n = append(buf, n, "== FIFO test == [ticks=");
    n = append_u(buf, n, (uint)uptime());
    n = append(buf, n, "]\n");
    write(1, buf, n);
  }

  int pid;

  // A llega primero
  pid = fork();
  if (pid == 0) cpu_work("A");
  puts1("A", "ARRIVAL", pid);
  sleep(3); // separa la llegada de B

  // B
  pid = fork();
  if (pid == 0) cpu_work("B");
  puts1("B", "ARRIVAL", pid);
  sleep(3); // separa la llegada de C

  // C
  pid = fork();
  if (pid == 0) cpu_work("C");
  puts1("C", "ARRIVAL", pid);

  // Esperar a todos
  while (wait(0) > 0) {}

  // Esta parte ahora está bien indentada
  {
    char buf[64]; int n=0;
    n = append(buf, n, "== FIFO test done == [ticks=");
    n = append_u(buf, n, (uint)uptime());
    n = append(buf, n, "]\n");
    write(1, buf, n);
  }

  exit(0);
}
