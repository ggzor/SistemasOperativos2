#include "productorConsumidor.h"
#include "vtime.h"

#include <getopt.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void imprimirUso() {
  printf("Uso: bin/main [-s semilla=0] [-t] <planificador> <lista-procesos>\n"
          "  Planificadores disponibles:\n"
          "    fifo: Planificador FIFO\n"
          "    sjf:  Trabajo más corto primero\n"
          "    rr:   Round-robin con quantum-variable por prioridad\n"
          "    srtf: El trabajo con el menor tiempo restante primero\n"
          "\n"
          "  Opciones:\n"
          "    -s    La semilla es para la generación de números aleatorios (0 por defecto)\n"
          "    -t    Realizar la simulación utilizando tiempo virtual, es decir\n"
          "          usando sólo contadores y no \"sleep\"\n");

  exit(-1);
}

void ejecutarComando(const char *comando);

#define MAX_LEN 100
/**
 *  Programa principal para ejecutar todos los procesos
 **/
int main(int argc, char **argv) {
  char comando[MAX_LEN];

  // Recepción de parámetros
  char op;
  char *planificador;
  char *lista;
  int semilla = 0;
  int tiempoVirtualActivo = 0;

  while ((op = getopt(argc, argv, ":s:t")) != -1) {
    switch (op)
    {
      case 's':
        semilla = atoi(optarg);
        break;
      case 't':
        tiempoVirtualActivo = 1;
        break;
      case ':':
        printf("ERROR: No se proporcionó un valor para la semilla.\n");
        imprimirUso();
      case '?':
        printf("ERROR: Opción no reconocida: %c\n", optopt);
        imprimirUso();
      default:
        imprimirUso();
    }
  }

  // Validar cantidad de argumentos
  argc -= optind;
  if (argc != 2) {
    printf("ERROR: No se proporcionaron los argumentos necesarios.\n");
    imprimirUso();
  }

  // Validar planificador existente
  planificador = argv[optind];
  if (!((strncmp("fifo", planificador, 4) == 0) || 
        (strncmp("sjf",  planificador, 3) == 0) ||
        (strncmp("rr",   planificador, 2) == 0) ||
        (strncmp("srtf", planificador, 4) == 0))) {
    printf("ERROR: El planificador \"%s\" no está disponible\n", planificador);
    imprimirUso();
  }

  lista = argv[optind + 1];

  // Limpieza de recursos dejados por ejecuciones anteriores
  limpiarRecursos();
  limpiarRecursosVTime();

  if (tiempoVirtualActivo == 0)
    printf("Usando tiempo real.\n");

  // Usar tiempo virtual si se requiere
  usarTiempoReal(tiempoVirtualActivo == 0);

  // Ejecución de comandos
  snprintf(comando, MAX_LEN, "bin/lectorProcesos %s %d", lista, semilla);
  ejecutarComando(comando);

  snprintf(comando, MAX_LEN, "bin/%s-largo %d", planificador, semilla);
  ejecutarComando(comando);

  snprintf(comando, MAX_LEN, "bin/%s-corto salida-%s-%s-%s.txt", 
            planificador, planificador, lista, tiempoVirtualActivo ? "vtime" : "real");
  ejecutarComando(comando);

  snprintf(comando, MAX_LEN, "bin/despachador");
  ejecutarComando(comando);

  // Esperar hijos
  while (wait(0) != -1);

  // Limpiar recursos antes de terminar el programa
  limpiarRecursos();
  limpiarRecursosVTime();

  return 0;
}

void ejecutarComando(const char *comando) {
  if (fork() == 0) {
    printf("Terminó con %d -- %s\n", system(comando), comando);
    _exit(0);
  }
}
