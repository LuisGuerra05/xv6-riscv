# Tarea 3: Protección de Lectura en XV6 (mrdprotect & munrdprotect)

**Nombres:** Luis Guerra y Alejandro Mañón  


## 1. Funcionamiento y lógica de la implementación

En esta sección se explica:

- El objetivo del mecanismo de protección de lectura.
- Cómo funcionan `mrdprotect()` y `munrdprotect()`.
- Cómo se recorre la tabla de páginas y cómo se modifica el bit **PTE_R**.
- Qué ocurre con las páginas afectadas (sin lectura / lectura restaurada).
- Evidencia visual con capturas de la consola mostrando los resultados del test.

Incluye aquí las imágenes del `rdprotect_test.c` ejecutándose y la explicación del comportamiento observado.


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

Un fragmento representativo del programa es el siguiente:

```c
char *addr = sbrk(0);
sbrk(4096);

addr[0] = 'X';     // OK

mrdprotect(addr, 1);

addr[0] = 'A';     // OK (escritura permitida)

char c = addr[0];  // PAGE FAULT ESPERADO
```

Este programa permite comprobar la semántica solicitada: la página puede escribirse aun cuando no puede leerse, y el permiso puede restaurarse exitosamente.





## 3. Dificultades encontradas y soluciones implementadas

Describe aquí los problemas que realmente viviste, por ejemplo:

- Direcciones no alineadas → solución.
- Page faults inesperados → solución.
- Errores al recorrer PTEs → solución.
- Problemas con syscalls → solución.
- Páginas sin `PTE_U` → solución.


## 4. Análisis: Riesgos, limitaciones y consideraciones de seguridad



## 5. Conclusiones

Resumir:

- Que las funciones funcionan correctamente.
- Que el mecanismo protege contra lectura.
- Que el test confirma los resultados.
- Que el código es robusto ante errores.
- Posibles mejoras o extensiones.

