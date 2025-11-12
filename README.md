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

### 2.1. Incorporar funciones de protección

Describe:

- En qué archivo se implementaron (`vm.c`).
- La firma de cada función:
  - `int mrdprotect(void *addr, int len)`
  - `int munrdprotect(void *addr, int len)`
- La lógica general de ambas funciones.
- Cómo se calcula la cantidad de páginas.
- Validaciones iniciales.
- Recorrido de los PTE.

### 2.2. Modificación de la Tabla de Páginas

Explica:

- Cómo se obtiene cada PTE con `walk()`.
- Validación de `PTE_V` y `PTE_U`.
- Modificación bitwise:
  - `mrdprotect` → limpia `PTE_R`
  - `munrdprotect` → activa `PTE_R`
- Mantener intactos otros bits (`W`, `X`, `U`, `V`).

### 2.3. Manejo de errores y robustez

Incluye todo lo que pide la pauta:

- Error si `addr` no está alineada.
- Error si `len <= 0`.
- Error si alguna página no pertenece al espacio de usuario.
- Error si alguna página no está mapeada (`!PTE_V`).
- Error si la operación involucra una página del kernel.
- Retornos `0` o `-1`.

### 2.4. Programa de prueba: `rdprotect_test.c`

Puedes dejarlo como **2.4** porque forma parte de las modificaciones y funcionalidad.

Aquí explicas:

- Cómo funciona el programa de prueba.
- Qué se espera que pase:
  - Escritura funciona siempre.
  - Lectura protegida → page fault.
  - Lectura restaurada → vuelve a funcionar.
- Incluye de nuevo capturas si quieres.


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

