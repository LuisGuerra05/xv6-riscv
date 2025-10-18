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


### 2.4 Implementación del Lottery Scheduler

**Archivo:** `kernel/proc.c`

Se reemplazó el algoritmo *Round-Robin* del scheduler por una versión basada en el principio de *Lottery Scheduling*, donde la probabilidad de que un proceso sea seleccionado para ejecutar es proporcional al número de tickets que posee. Para ello, se modificó la función `scheduler()` incorporando un cálculo del total de tickets de los procesos en estado `RUNNABLE`, la generación de un número aleatorio dentro de ese rango, y la selección del proceso ganador mediante una acumulación de tickets hasta alcanzar el valor generado. Una vez elegido, el proceso pasa a estado `RUNNING` y se incrementa su contador `run_slices`. 

Adicionalmente, se implementó una pequeña función generadora de números pseudoaleatorios (`random()`) en el mismo archivo, utilizada para determinar el proceso ganador de cada iteración. Se añadieron también comentarios explicativos en el código para documentar el funcionamiento del nuevo scheduler.

---

### 2.5 Contabilidad y Monitoreo

**Archivos modificados:** `kernel/proc.h`, `kernel/proc.c`

Para evaluar el comportamiento del scheduler, se aprovechó el campo `run_slices`, el cual se incrementa cada vez que un proceso es seleccionado para ejecutarse. Con esta métrica, es posible verificar la proporcionalidad entre la cantidad de tickets asignados y el número de veces que el proceso fue planificado. 

Asimismo, se modificó la función `procdump()` (invocable desde la consola con **Ctrl+P**) para mostrar los valores de `tickets` y `run_slices` de cada proceso. Esto permite observar en tiempo real cómo los procesos con más tickets tienden a recibir más tiempo de CPU, validando empíricamente el comportamiento esperado del algoritmo de lotería.

---

### 2.6 Programa de Prueba `demo.c`

**Archivos modificados:** `user/demo.c`, `Makefile`

Se desarrolló un programa de usuario denominado `demo.c` que crea múltiples procesos (10 en total) mediante llamadas a `fork()`. A cada proceso se le asigna un número distinto de tickets utilizando la syscall `settickets(int n)`. Los procesos realizan una carga de trabajo intensiva en CPU, lo que permite al scheduler distribuir equitativamente el uso del procesador de acuerdo con las probabilidades definidas por los tickets. 

Para compilarlo junto con el resto del sistema, se añadió el ejecutable `_demo` a la lista `UPROGS` del `Makefile`. Al ejecutar el comando `demo` dentro de xv6 y posteriormente presionar **Ctrl+P**, se puede observar en la salida del sistema cómo los procesos con mayor cantidad de tickets fueron seleccionados con más frecuencia, confirmando el correcto funcionamiento del *Lottery Scheduler*.

Con estas últimas modificaciones, el sistema xv6 implementa de forma completa el *Lottery Scheduler*, junto con sus mecanismos de contabilidad, monitoreo y validación empírica.


## 3. Dificultades encontradas y soluciones implementadas




## 4. Posibles problemas de este tipo de Scheduler (Lottery Scheduling)

En esta sección se debe analizar críticamente el *Lottery Scheduling* y mencionar las limitaciones o desventajas que presenta este enfoque, por ejemplo:
- Posible injusticia a corto plazo (procesos con pocos tickets pueden no ejecutarse durante largo tiempo)
- Variabilidad aleatoria en los tiempos de respuesta
- Complejidad de implementación o depuración
- Ineficiencia en sistemas con pocos procesos o alto cambio de contexto

Se pueden incluir ejemplos o reflexiones sobre cuándo este tipo de scheduler es apropiado o cuándo no lo sería, comparándolo con otros algoritmos como *Round-Robin*, *Priority Scheduling* o *MLFQ*.

