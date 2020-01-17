#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  // Recepción de parámetros
  char *planificador;
  char *lista;
  int semilla = 0;

  if (!(argc == 3 || argc == 4)) {
    printf("Uso: bin/main <fifo|sjf|rr> <lista-procesos> [semilla=0]\n"
           "  Planificadores disponibles:\n"
           "    - fifo: Planificador FIFO\n"
           "    - sjf:  Trabajo más corto primero\n"
           "    - rr:   Round-robin con quantum-variable por prioridad\n"
           "\n"
           "  Notas:\n"
           "    - La semilla es para la generación de números aleatorios en el\n"
           "      proceso que lee los procesos\n");
    exit(-1);
  }


  planificador = argv[1];
  if (!((strncmp("fifo", planificador, 4) == 0) || 
        (strncmp("sjf",  planificador, 3) == 0) ||
        (strncmp("rr",   planificador, 2) == 0))) {
    printf("El planificador \"%s\" no existe. Solo puede ser uno de los siguientes:\n"
           "  - fifo, sjf o rr\n");
    exit(-1);
  }

  lista = argv[2];

  if (argc == 4)
    semilla = atoi(argv[3]);

  // Limpieza de recursos
  if (semctl(semget(0x12, 0, 0666), 0, IPC_RMID) >= 0)
    printf("Se borró el semáforo de la ejecución anterior.\n");

  if (shmctl(shmget(0x13, 4, 0666), IPC_RMID, 0) >= 0)
    printf("Se borró el segmento de memoria compartida de la ejecución anterior.\n");

  // Ejecución de comandos
  snprintf(comando, MAX_LEN, "bin/lectorProcesos %s %d", lista, semilla);
  ejecutarComando(comando);

  snprintf(comando, MAX_LEN, "bin/%s-largo %d", planificador, semilla);
  ejecutarComando(comando);

  snprintf(comando, MAX_LEN, "bin/%s-corto %d", planificador, semilla);
  ejecutarComando(comando);

  ejecutarComando("bin/despachador");

  // Esperar hijos
  while (wait(0) != -1);

  return 0;
}
