#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int n = 10;
  int pids[10];
  int tickets[10];

  printf("Iniciando demo del Lottery Scheduler con %d procesos...\n", n);

  // Crear hijos
  for (int i = 0; i < n; i++) {
    int pid = fork();
    if (pid == 0) {
      // Hijo
      int t = 50 * (i + 1);
      settickets(t);

      // Esperar un poco para no saturar consola
      sleep(i * 10);

      // Simular carga de CPU
      for (int k = 0; k < 5; k++) {
        for (volatile int j = 0; j < 100000000; j++);
        sleep(10);
      }

      exit(0);
    } else {
      // Padre guarda info
      pids[i] = pid;
      tickets[i] = 50 * (i + 1);
    }
  }

  // Esperar un tiempo antes de que los hijos terminen
  sleep(300);

  // 🔹 Aquí imprime solo una vez todos los resultados
  printf("\n--- Resultados de asignación ---\n");
  for (int i = 0; i < n; i++)
    printf("Proceso hijo PID=%d asignado con %d tickets\n", pids[i], tickets[i]);

  printf("\nDemo completada.\nEjecutando printslices() para verificar proporción de ejecución...\n");
  printslices();

  // Esperar a que terminen
  for (int i = 0; i < n; i++)
    wait(0);

  printf("\nFin de la demo.\n");
  exit(0);
}
