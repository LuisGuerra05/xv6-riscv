# INFORME – Tarea 1: Implementación de Llamadas al Sistema en xv6

**Nombres: Luis Guerra y Alejandro Mañón**

## 1. Funcionamiento de las llamadas al sistema

- **getppid(void)**: retorna el identificador de proceso (PID) del padre del proceso que la invoca.  
  Ejemplo: si el shell (`sh`) es PID=2 y ejecuta un programa con PID=3, entonces `getppid()` desde el hijo devolverá `2`.  

- **getancestor(int n)**: retorna el PID del ancestro del proceso según el valor de `n`:  
  - `getancestor(0)`: el mismo proceso.  
  - `getancestor(1)`: el padre.  
  - `getancestor(2)`: el abuelo.  
  - Si no existe el ancestro, devuelve `-1`.  

Estas llamadas permiten a un proceso conocer la jerarquía de procesos en xv6 y fueron probadas con el programa `yosoytupadre.c`.


## 2. Explicación de las modificaciones realizadas

Para implementar estas llamadas al sistema se siguió el mismo patrón que la syscall existente `getpid`. El trabajo se dividió en varios pasos y archivos:

### 2.1. Definir números de syscall
En `kernel/syscall.h` se agregó:
```c
#define SYS_getppid     22
#define SYS_getancestor 23
```

### 2.2. Declarar y mapear funciones en la tabla de syscalls

En `kernel/syscall.c`:
```c
extern uint64 sys_getppid(void);
extern uint64 sys_getancestor(void);

static uint64 (*syscalls[])(void) = {
  // ...
  [SYS_getppid]     sys_getppid,
  [SYS_getancestor] sys_getancestor,
};
```

### 2.3. Implementación en el kernel

En `kernel/sysproc.c`:
```c
uint64 sys_getppid(void) {
  struct proc *p = myproc();
  return p->parent ? p->parent->pid : -1;
}

uint64 sys_getancestor(void) {
  int n;
  if (argint(0, &n) < 0 || n < 0) return -1;
  struct proc *cur = myproc();
  while (n-- > 0 && cur) {
    cur = cur->parent;
  }
  return cur ? cur->pid : -1;
}
```

### 2.4. Prototipos en espacio de usuario

En `user/user.h`:
```c
int getppid(void);
int getancestor(int);
```

### 2.5. Stubs de usuario

En `user/usys.pl`:
```c
entry("getppid");
entry("getancestor");
```

### 2.6. Programa de prueba

Se creó `user/yosoytupadre.c`:

```c
#include "kernel/types.h"
#include "user/user.h"

int main() {
  printf("PID=%d, PPID=%d\n", getpid(), getppid());
  printf("Anc(0)=%d, Anc(1)=%d, Anc(2)=%d\n",
          getancestor(0), getancestor(1), getancestor(2));
  exit(0);
}
```

En el Makefile, se agregó a la lista UPROGS:

```make
$U/_yosoytupadre
```

### 2.7. Compilación y ejecución

Se recompiló xv6 con:

```bash
make clean && make qemu
```

Y se probó el programa en la shell de xv6:

```bash
$ yosoytupadre
PID=3, PPID=2
Anc(0)=3, Anc(1)=2, Anc(2)=1
```

## 3. Dificultades encontradas y cómo se resolvieron.