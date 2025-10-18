# Tarea 2: Implementación de Scheduler Lottery en xv6

**Nombres:** Luis Guerra y Alejandro Mañón  



## 1. Funcionamiento y lógica de la implementación

En esta sección se debe describir el principio de funcionamiento del *Lottery Scheduler* implementado en xv6.  
Se debe explicar cómo se asignan los tickets a cada proceso, cómo se realiza el sorteo (selección aleatoria) y cómo se determina qué proceso se ejecuta en cada turno del CPU.  





## 2. Explicación de las modificaciones realizadas

Para implementar el *Lottery Scheduler* en xv6 se realizaron modificaciones en distintos archivos del kernel y del espacio de usuario. A continuación se describen los cambios clave y su propósito.

### 2.1 Agregar campos `tickets` y `run_slices` en `proc.h`

**Archivo:** `kernel/proc.h`

Se añadieron dos nuevos campos a la estructura `struct proc`:

```c
int tickets;      // cantidad de tickets del proceso (mínimo 1)
int run_slices;   // cantidad de veces que el proceso fue elegido para ejecutarse
```

Estos valores permiten registrar la cantidad de tickets que posee cada proceso (proporcional a su probabilidad de ser seleccionado) y contabilizar cuántas veces fue efectivamente ejecutado por el scheduler.

### 2.2 Inicializar valores por defecto en allocproc()

**Archivo**: `kernel/proc.c`

En la función `allocproc()` se inicializan los campos recién agregados para cada nuevo proceso:

```c
p->tickets = 100;    // valor inicial por defecto
p->run_slices = 0;   // contador de ejecuciones
```

De esta forma, todos los procesos nuevos comienzan con 100 tickets por defecto, asegurando igualdad de condiciones iniciales.


### 2.3 Creación de la syscall settickets(int n)

**Archivos modificados**:

- `kernel/sysproc.c`
- `kernel/syscall.c`
- `kernel/syscall.h`
- `user/user.h`
- `user/usys.pl`

**Cambios realizados**:

1. `sysproc.c` → se implementó la función que modifica los tickets del proceso actual:

```c
uint64
sys_settickets(void)
{
  int n;
  argint(0, &n);
  struct proc *p = myproc();
  if (n < 1)
    n = 1;
  p->tickets = n;
  return 0;
}
```

2. `syscall.h` → se asignó un nuevo número de syscall:

```c
#define SYS_settickets 22
```


3. `syscall.c` → se declaró y mapeó la nueva syscall:
```c
extern uint64 sys_settickets(void);

...

[SYS_settickets] sys_settickets,
```


4. `user/user.h` → se agregó el prototipo visible para programas de usuario:
```c
int settickets(int);
```

5. `user/usys.pl` → se añadió la entrada correspondiente:
```c
entry("settickets");
```

Esta syscall permite que un proceso modifique dinámicamente su cantidad de tickets de CPU.


### 2.4. Implementación del Lottery Scheduler




## 3. Dificultades encontradas y soluciones implementadas




## 4. Posibles problemas de este tipo de Scheduler (Lottery Scheduling)

En esta sección se debe analizar críticamente el *Lottery Scheduling* y mencionar las limitaciones o desventajas que presenta este enfoque, por ejemplo:
- Posible injusticia a corto plazo (procesos con pocos tickets pueden no ejecutarse durante largo tiempo)
- Variabilidad aleatoria en los tiempos de respuesta
- Complejidad de implementación o depuración
- Ineficiencia en sistemas con pocos procesos o alto cambio de contexto

Se pueden incluir ejemplos o reflexiones sobre cuándo este tipo de scheduler es apropiado o cuándo no lo sería, comparándolo con otros algoritmos como *Round-Robin*, *Priority Scheduling* o *MLFQ*.

