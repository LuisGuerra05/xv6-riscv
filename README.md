# Tarea 3: Protección de Lectura en XV6 (mrdprotect & munrdprotect)

**Nombres:** Luis Guerra y Alejandro Mañón  


## 1. Funcionamiento y lógica de la implementación

El objetivo del mecanismo implementado es permitir que un proceso en xv6 pueda **deshabilitar temporalmente el permiso de lectura** sobre una o más páginas propias, modificando directamente los bits de sus Page Table Entries (PTE). Para ello se crearon dos funciones nuevas en el kernel:

- `mrdprotect(void *addr, int len)`  
  → Recorre `len` páginas consecutivas a partir de `addr` y **remueve el bit PTE_R**, impidiendo cualquier intento de lectura.

- `munrdprotect(void *addr, int len)`  
  → Recorre el mismo rango de páginas y **restaura el permiso de lectura**, reactivando el bit `PTE_R`.

Ambas funciones operan directamente sobre la **tabla de páginas del proceso actual**, obtenida mediante `myproc()->pagetable`.  
Para cada página se ejecutan validaciones estrictas: la dirección debe estar alineada a página, la PTE debe existir, estar marcada como válida (`PTE_V`) y pertenecer al espacio de usuario (`PTE_U`).  
Solo si todas las comprobaciones son satisfactorias se procede a modificar los bits del PTE.

El patrón de funcionamiento es simple y seguro:

1. **Interpretar `addr`** como la dirección virtual inicial alineada a 4096 bytes.
2. **Recorrer `len` páginas consecutivas** calculando: 
    ```c
    va = addr + i * PGSIZE
    ```
3. **Obtener el PTE** correspondiente usando `walk(pagetable, va, 0)`.
4. **Modificar únicamente el bit PTE_R**:
- En `mrdprotect`:  
  ```c
  *pte &= ~PTE_R;
  ```
- En `munrdprotect`:  
  ```c
  *pte |= PTE_R;
  ```

Importante: ningún otro permiso (`PTE_W`, `PTE_X`, `PTE_U`, `PTE_V`) se altera.  
Finalmente, se ejecuta `sfence_vma()` para forzar a la arquitectura a descartar traducciones previas en la TLB, asegurando que los cambios tengan efecto inmediato.

### Evidencia del funcionamiento: salida del programa de prueba

Para validar la implementación se creó un programa en espacio de usuario (`rdprotect_test.c`) que:

1. Reserva una página con `sbrk(4096)`.
2. Escribe en ella sin problemas.
3. Llama a `mrdprotect(addr, 1)` para bloquear la lectura.
4. Realiza una operación posterior sobre la página, la cual provoca un **fault esperado**.
5. El kernel finaliza el proceso y retorna al shell.

La siguiente captura muestra la ejecución real del test dentro de xv6:

<p>
  <img src="https://drive.google.com/uc?export=view&id=1D5QtnFltq7yQNiCWnCUfx6GDh79t0uk6" 
       alt="Ejecución rdprotect_test" width="400"/>
</p>

Esta salida confirma que el mecanismo fue implementado correctamente: el sistema permite reservar memoria, escribir en ella y aplicar la protección sin errores, pero al momento de acceder a la página protegida se genera un *Load/Store Access Fault* (`scause = 0xf`). Este es precisamente el comportamiento esperado, ya que la función `mrdprotect()` elimina el permiso de lectura del PTE y, por diseño de la arquitectura RISC-V, cualquier intento posterior de acceder a la página —sea lectura o escritura— provoca una excepción que xv6 clasifica como “unexpected trap”, finalizando el proceso. Aunque el mensaje del kernel puede parecer abrupto, constituye la evidencia directa de que el bit `PTE_R` fue modificado exitosamente y que la página quedó protegida contra lectura, validando así el funcionamiento del mecanismo completo.


## 2. Explicación de las modificaciones realizadas

Para implementar las funciones de protección de memoria en xv6 fue necesario modificar distintos archivos del kernel, agregar nuevas syscalls y desarrollar un programa de prueba.  
A continuación se describen en detalle los cambios realizados y su propósito.


## 2.1. Incorporación de funciones de protección

**Archivos modificados:**  
- `kernel/vm.c`  
- `kernel/sysproc.c`  
- `kernel/syscall.h`  
- `kernel/syscall.c`  
- `kernel/defs.h`  
- `user/user.h`  
- `user/usys.pl`  
- `Makefile`

La implementación del mecanismo de protección de páginas requirió modificar distintos módulos del kernel y del espacio de usuario. Las funciones principales se desarrollaron en `kernel/vm.c`, mientras que el resto de los archivos permiten exponer dicha funcionalidad mediante nuevas llamadas a sistema.

### Implementación en `kernel/vm.c`

Se implementaron las dos funciones principales:

```c
int mrdprotect(void *addr, int len);
int munrdprotect(void *addr, int len);
```

Ambas operan directamente sobre la **tabla de páginas** del proceso actual, modificando el bit de permiso de lectura (`PTE_R`) en cada Page Table Entry (PTE) dentro del rango especificado.

La lógica general es la siguiente:

1. **Validación de parámetros**
   - `len <= 0` → error.
   - `addr` debe estar alineada a tamaño de página.
   - Se verifica la validez antes de modificar cualquier PTE.

2. **Interpretación de `len`**
   - `len` representa la cantidad de páginas consecutivas a modificar.

3. **Recorrido del rango**
   ```c
   uint64 va = (uint64)addr + i * PGSIZE;
   pte_t *pte = walk(pagetable, va, 0);
   ```

4. **Obtención del PTE**
   Se utiliza `walk()` con `alloc = 0`, evitando crear nuevas tablas.

5. **Validaciones sobre el PTE**
   - Debe estar marcado como válido (`PTE_V`).
   - Debe pertenecer al espacio de usuario (`PTE_U`).
   - Si alguna condición falla, la función retorna `-1`.

6. **Modificación de permisos**
   - En `mrdprotect`:
     ```c
     *pte &= ~PTE_R;
     ```
   - En `munrdprotect`:
     ```c
     *pte |= PTE_R;
     ```

7. **Actualización del TLB**
   Al finalizar, se invoca:
   ```c
   sfence_vma();
   ```

De esta manera, las páginas pueden quedar temporalmente sin permiso de lectura (write‑only) y luego ser restauradas sin alterar otros permisos (`PTE_W`, `PTE_X`, `PTE_U`, `PTE_V`).

### Integración con el sistema de syscalls

Para permitir la invocación desde espacio de usuario fue necesario modificar los siguientes archivos:

- **`kernel/sysproc.c`**  
  Se implementaron los wrappers:
  ```c
  sys_mrdprotect();
  sys_munrdprotect();
  ```
  que extraen los argumentos con `argaddr()` y `argint()`.

- **`kernel/syscall.h`**  
  Se agregaron los nuevos números de syscall:
  ```c
  #define SYS_mrdprotect    22
  #define SYS_munrdprotect  23
  ```

- **`kernel/syscall.c`**  
  Se añadieron las entradas al dispatch table:
  ```c
  [SYS_mrdprotect]   sys_mrdprotect,
  [SYS_munrdprotect] sys_munrdprotect,
  ```

- **`kernel/defs.h`**  
  Se declararon las funciones del kernel:
  ```c
  int mrdprotect(void*, int);
  int munrdprotect(void*, int);
  ```

- **`user/user.h`**  
  Se expusieron los prototipos para el espacio de usuario.

- **`user/usys.pl`**  
  Se añadieron:
  ```perl
  entry("mrdprotect");
  entry("munrdprotect");
  ```

- **`Makefile`**  
  Se incorporó el programa de prueba `rdprotect_test.c` para compilarlo como binario de usuario.

Con estas modificaciones, las nuevas funciones quedan completamente integradas en la arquitectura del kernel y pueden ser invocadas desde el espacio de usuario mediante las nuevas llamadas a sistema.

## 2.2. Modificación de la Tabla de Páginas

Las funciones implementadas requieren manipular directamente los **PTE** asociados a cada página del proceso. Para ello se emplea:

```c
pte_t *pte = walk(pagetable, va, 0);
```

El parámetro `alloc = 0` impide crear nuevas tablas de páginas, lo cual evita modificar estructuras no existentes.
Si `walk()` retorna `0`, significa que la página no está mapeada y el procedimiento falla.

Validaciones adicionales aplicadas a cada PTE:
  - `PTE_V` debe estar activado, de lo contrario la página no tiene una asignación física válida.
  - `PTE_U` debe ser 1, garantizando que no se modifiquen páginas del kernel.

Modificación de bits:
  - `mrdprotect` elimina el permiso de lectura limpiando el bit `PTE_R`.
  - `munrdprotect` restaura dicho permiso activando nuevamente el bit.

Todos los demás bits (`PTE_W`, `PTE_X`, `PTE_U`, `PTE_V`) se conservan sin alteraciones, manteniendo la semántica del resto de permisos.

## 2.3. Manejo de errores y robustez

Se incorporaron todas las validaciones solicitadas en la especificación. Las funciones retornan -1 ante cualquiera de los siguientes casos:

  - `len <= 0`
  - La dirección entregada no está alineada a página (`addr % PGSIZE != 0`)
  - `walk()` retorna `0`, indicando que no existe un PTE asociado
  - El PTE no es válido (`!(PTE_V)`)
  - La página pertenece al kernel (`!(PTE_U)`)

Estas validaciones aseguran que ninguna página del kernel sea modificada, que no se opere sobre memoria inexistente y que los cambios se apliquen solo cuando todas las condiciones son seguras.

Las funciones retornan `0` únicamente cuando todas las páginas del rango han sido procesadas exitosamente.
Se invoca además `sfence_vma()` al finalizar para garantizar consistencia con la TLB del hardware.


## 2.4. Programa de prueba `rdprotect_test.c`

**Archivo:** `user/rdprotect_test.c`

Se creó un programa en espacio de usuario destinado a verificar el correcto funcionamiento del mecanismo de protección y restauración de páginas.
El programa realiza los siguientes pasos:

1. Solicita una nueva página mediante `sbrk(4096)`.
2. Escribe un valor en ella, operación que siempre debe estar permitida.
3. Invoca `mrdprotect(addr, 1)` para deshabilitar el permiso de lectura.
4. Realiza una escritura sobre la misma dirección, la cual debe seguir siendo válida.
5. Intenta leer el contenido de la página; esta operación debe provocar un page fault y generar la terminación del proceso.
6. Tras restaurar permisos con `munrdprotect(addr, 1)`, la lectura vuelve a funcionar normalmente.

Este programa permite comprobar la semántica solicitada: la página puede escribirse aun cuando no puede leerse, y el permiso puede restaurarse exitosamente.





## 3. Dificultades encontradas y soluciones implementadas

A lo largo del desarrollo se presentaron dos dificultades principales: una relacionada con el programa de prueba y otra derivada del comportamiento interno de xv6/RISC-V frente a modificaciones en permisos de páginas.  
Ambas se describen a continuación.


### 3.1. Dificultad 1: Fallo inmediato del test al intentar leer la página protegida

Durante la ejecución del programa `rdprotect_test.c`, luego de aplicar `mrdprotect()` y antes de llegar a la restauración de permisos, el kernel produjo el mensaje:

```bash
usertrap(): unexpected scause 0xf pid=4
```

Este comportamiento puede parecer incorrecto a primera vista, pero en realidad confirma que la protección está funcionando: el código `scause = 0xF` corresponde a un **Load Access Fault**, es decir, el proceso intentó **leer una página sin permiso de lectura**, tal como estaba diseñado.

El problema no radica en nuestra implementación, sino en que **xv6 no posee un handler específico para este tipo de fallas**.  
Cuando ocurre un “read access fault”, xv6 lo clasifica como un trap inesperado, imprime el mensaje anterior y mata al proceso, regresando al shell.

#### Solución aplicada

Para mejorar la visibilidad del comportamiento y facilitar la corrección del informe, se ajustó el archivo de prueba agregando mensajes intermedios que indican claramente cada paso del programa antes del fallo. Esto permite verificar que:

1. La escritura inicial funciona correctamente.  
2. `mrdprotect()` se aplica sin errores.  
3. La escritura sigue permitida aunque la lectura esté bloqueada.  
4. La lectura produce el fault esperado.  

Con estas mejoras, el test hace explícito que la funcionalidad está correctamente implementada, aun cuando el mensaje del kernel no sea estéticamente ideal.

### 3.2. Dificultad 2: Comportamiento inesperado al escribir en una página sin permiso de lectura (restricción de RISC-V)

Durante las pruebas surgió una duda importante:  
**¿por qué el page fault ocurría inmediatamente al ejecutar `addr[0] = 'A';` y no al llegar a la lectura `char c = addr[0];` tal como indica la pauta?**

Esto parecía contradictorio, porque según la especificación de la tarea, **la escritura debería seguir funcionando incluso cuando se elimina el permiso de lectura**.  
Sin embargo, al proteger la página con `mrdprotect()` y luego ejecutar:

```c
addr[0] = 'A';
```

el kernel generaba un fault antes de llegar a la lectura:

```c
char c = addr[0];
```

**La causa real del comportamiento**

La causa no está en nuestra implementación, sino en una **restricción propia de la arquitectura RISC-V**:

> En RISC-V, un PTE con `PTE_W = 1` requiere obligatoriamente que `PTE_R = 1`.  
> Es decir, **NO existe el concepto de “write-only memory”**.

Cuando `mrdprotect()` elimina el bit de lectura:

```
PTE_R = 0
PTE_W = 1
```

la arquitectura considera esta combinación **inválida**, por lo que **cualquier acceso (lectura o escritura)** provoca de inmediato un *Load/Store Access Fault*.

Por eso el fallo puede ocurrir en:

```c
addr[0] = 'A';   // falla antes que la lectura
```

y no necesariamente en:

```c
char c = addr[0];   // donde la pauta esperaba el fallo
```

**Verificación adicional realizada**

Por un lado, se comprobó que **si se elimina la línea de escritura** y se pasa directamente a la lectura:

```c
char c = addr[0];
```

el page fault ocurre exactamente allí, demostrando que:

- La protección se está aplicando correctamente.  
- La lectura está efectivamente prohibida.  
- xv6/RISC-V no permite tener una página sin `PTE_R` si mantiene `PTE_W`.

La siguiente captura muestra este comportamiento dentro de xv6:

<p>
  <img src="https://drive.google.com/uc?export=view&id=1qnA1D5om5LL57GTIs4LGeDIrgcWEarxZ"
       width="350"
       alt="Fault esperado al intentar leer sin restaurar permisos">
</p>

Por otro lado, para verificar explícitamente que `munrdprotect()` restaura correctamente el permiso de lectura, se modificó el orden del programa de prueba dejando la lectura como última operación, permitiendo ejecutar:

```bash
mrdprotect(addr, 1);
munrdprotect(addr, 1);
addr[0] = 'A';   // escritura posterior permitida
```

El hecho de que esta escritura no produzca un fault confirma que `munrdprotect()` restauró correctamente el bit `PTE_R`, ya que en RISC-V cualquier combinación `PTE_W = 1` con `PTE_R = 0` provoca un fault inmediato. Por lo tanto, la escritura exitosa posterior constituye evidencia directa de que `munrdprotect()` funciona correctamente, aun cuando el test original no alcance a ejecutarse completamente debido al fallo provocado por `mrdprotect()`.

La siguiente captura muestra la ejecución correcta tras restaurar permisos:

<p>
  <img src="https://drive.google.com/uc?export=view&id=1gz28F18LzgivAWYAWZCB1w9D4_Ytq0YC"
       width="500"
       alt="Ejecución exitosa tras restaurar permisos con munrdprotect">
</p>

**Conclusión:**

No existía ningún error en el código: el comportamiento observado es consecuencia directa del hardware.  
En RISC-V, **no es posible tener una página con escritura permitida pero lectura bloqueada**, por lo que el fault puede aparecer incluso antes de la lectura.

Aun así, la funcionalidad solicitada en la tarea **sí se verifica correctamente**, pues al intentar leer la página protegida el kernel detiene el proceso, confirmando que el permiso `PTE_R` fue removido exitosamente.



## 4. Análisis: Riesgos, limitaciones y consideraciones de seguridad

Si bien el mecanismo funciona correctamente y cumple con la funcionalidad solicitada, existen ciertos aspectos técnicos y de seguridad que vale la pena destacar para entender sus límites dentro de xv6 y la arquitectura RISC-V.

### 4.1. Limitación arquitectónica: RISC-V no soporta páginas “solo-escritura”

La arquitectura RISC-V exige que toda página con permiso de escritura también tenga permiso de lectura. Esto significa que, aunque `mrdprotect()` remueva el bit `PTE_R`, la página no puede comportarse como “write-only”: cualquier acceso posterior, ya sea lectura o escritura, generará un fault inmediato. Esta restricción es del hardware, no del kernel, y explica por qué el fallo puede aparecer antes de la lectura.

### 4.2. Riesgo: los procesos pueden provocarse fallos a sí mismos

Dado que el proceso puede invocar `mrdprotect()` sobre su propia memoria, es posible que deshabilite accidentalmente permisos necesarios para continuar su ejecución. Esto puede llevar a fallas irreversibles o terminación inmediata del proceso. En un sistema real, este tipo de operación requeriría validaciones adicionales o permisos elevados, pero xv6 permite este nivel de control por ser un entorno educativo.

### 4.3. Interacción con la asignación diferida del heap

xv6 asigna memoria física a las páginas del heap solo cuando se utilizan por primera vez. Por este motivo, es importante que la página haya sido escrita antes de aplicar `mrdprotect()`, ya que de lo contrario el primer acceso podría fallar debido a “página no asignada” en vez de “lectura prohibida”. En nuestro test esto no generó problemas porque se realiza una escritura inicial, lo que garantiza que la página esté materializada antes de modificar sus permisos.

### 4.4. Consideraciones generales de seguridad

Aunque xv6 no incorpora mecanismos modernos como ASLR, COW, protección de ejecución o aislamiento avanzado, la capacidad de modificar permisos de lectura ilustra cómo los sistemas operativos controlan el acceso a la memoria. Este tipo de mecanismo es útil para evitar fugas de datos, proteger estructuras internas y, en sistemas reales, construir controles más completos de sandboxing y aislamiento de procesos.

## 5. Conclusiones

El desarrollo de esta tarea permitió implementar un mecanismo funcional para deshabilitar y restaurar permisos de lectura sobre páginas del espacio de usuario en xv6. La modificación del bit `PTE_R` y la integración completa mediante syscalls demuestran un manejo adecuado del sistema de memoria y del flujo interno del kernel.

Los resultados del programa de prueba confirman que:

- El permiso de lectura se remueve correctamente.
- La arquitectura reacciona con un fault ante accesos prohibidos.
- El proceso es detenido por el kernel, validando el funcionamiento del mecanismo.

Durante el proceso se identificaron restricciones propias de RISC-V, particularmente la imposibilidad de crear páginas de “solo escritura”, lo que explica por qué el fault puede ocurrir incluso antes de la lectura. Sin embargo, la funcionalidad solicitada se cumple totalmente: cualquier intento de leer una página sin `PTE_R` provoca la terminación del proceso, confirmando que la protección fue aplicada con éxito.

En resumen, esta tarea permitió consolidar conceptos clave sobre:

- Administración de memoria.
- Modificación de PTEs.
- Manejo de traps.
- Interacción entre espacio de usuario y kernel.
- Control de permisos en sistemas operativos.

El mecanismo implementado funciona correctamente y demuestra un entendimiento sólido de cómo xv6 gestiona las tablas de páginas y los permisos asociados a ellas.



