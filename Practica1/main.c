#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void ejecutarComando(const char *comando) {
  if (fork() == 0) {
    printf("Terminó con %d -- %s\n", system(comando), comando);
    _exit(0);
  }
}

#define MAX_LEN 100
/**
 *  Programa principal para ejecutar todos los procesos
 **/
int main(int argc, char **argv) {
  char comando[MAX_LEN];
  if (argc <= 1) {
    printf("Uso: ./main <fifo|sjf|rr>\n"
           "  fifo - Planificador FIFO\n"
           "  sjf  - Trabajo más corto primero\n");
    exit(-1);
  }

  // Limpieza de semáforo
  if (semctl(semget(0x12, 0, 0666), 0, IPC_RMID) >= 0)
    printf("Se borró el semáforo de la ejecución anterior.\n");

  ejecutarComando("bin/lectorProcesos test");

  snprintf(comando, MAX_LEN, "bin/%s-largo", argv[1]);
  ejecutarComando(comando);

  snprintf(comando, MAX_LEN, "bin/%s-corto", argv[1]);
  ejecutarComando(comando);

  ejecutarComando("bin/despachador");

  // Esperar hijos
  while (wait(0) != -1);

  return 0;
}
