// Propias 
#include "pipes.h"
#include "tipos.h"
#include "utilerias.h"
#include "vtime.h"

// No estandares
#include <unistd.h>
#include <sys/stat.h>

// Estandares
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Archivo de procesos
#define PATH_NAME_FILE "test" 

// Pipe entre lector de procesos y Planificador de largo plazo
#define RECEPTOR "/tmp/pipeReceptor"

#define MAX_PROCESOS 15
#define MIN_PROCESOS 1
#define MAX_TEMPORIZADOR 5
#define MIN_TEMPORIZADOR 1
#define DEBUG   

/* Aspectos del programa 
    Rafaga de proceso: 1-15 procesos
    Tiempos entre rafagas de procesos:  1-5 seg.
*/

int recoleccionProcesos(FILE * file, int numeroProcesos,Proceso *listaProcesos){
  int procesosLeidos = 0, i;
  while(procesosLeidos < numeroProcesos && (fscanf(file, "%d %d %d %d", 
                                                          &listaProcesos[procesosLeidos].nombre,
                                                          &listaProcesos[procesosLeidos].tiempo,
                                                          &listaProcesos[procesosLeidos].prioridad,
                                                          &listaProcesos[procesosLeidos].cantidadPag)) > 0){
    for (i = 0; i < CADENA_REF_LEN; i++) {
      fscanf(file, "%d", &listaProcesos[procesosLeidos].cadenaReferencias[i]);
    }

    listaProcesos[procesosLeidos].inicio = vtime();
    fgetc(file);
    procesosLeidos++;
  }

  return procesosLeidos; 
}

void determinarAleatorios(int *tiempoDormir, int *numeroProcesos){
  *tiempoDormir = rand() % MAX_TEMPORIZADOR + MIN_TEMPORIZADOR;
  *numeroProcesos = rand() % MAX_PROCESOS + MIN_PROCESOS;
}

void envioProcesos(int pd, Proceso *listaProcesos, int numeroProcesos){
  size_t data_write = write(pd, listaProcesos, sizeof(Proceso)*numeroProcesos );

  #ifdef DEBUG
    printf("Numero de procesos enviados en la rafaga: %d\n", numeroProcesos);
    for(int i =0; i<numeroProcesos; i++)
        printf("%d %d %d %d\n",
               listaProcesos[i].nombre,
               listaProcesos[i].tiempo,
               listaProcesos[i].prioridad,
               listaProcesos[i].cantidadPag);
  #endif

  if(data_write != sizeof(Proceso)*numeroProcesos)
    terminarProcesos("Error en el envio de procesos");
}

int main(int argc, char **argv){
  Proceso listaProcesos[MAX_PROCESOS];
  FILE * file = NULL;
  int pd = 0;
  int tiempoDormir = 0;
  int numeroProcesos = 0;
  int semilla = 0;

  if(argc != 3){
    printf("Uso: ./lectorProceso <lista-procesos> [semilla=0]\n"
           "  Recolecta la información de los proceso recolectados en el lectorProcesos\n");
    terminarProcesos("");
  }

  semilla = atoi(argv[2]);

  srand(semilla);

  file = fopen(argv[1], "r");
  if(file == NULL)
      terminarProcesos("Error en la apertura del archivo de procesos - ");

  pd = abrirPipeEscritura(RECEPTOR);
  do{
    determinarAleatorios(&tiempoDormir, &numeroProcesos);
    numeroProcesos = recoleccionProcesos(file, numeroProcesos, listaProcesos);

    if (numeroProcesos != 0){
        envioProcesos(pd, listaProcesos, numeroProcesos);
        vsleep(tiempoDormir);
    }
  } while(!feof(file));

  fclose(file);
  // Cierre de pipe determina fin de la tranmision de proceso hacia el receptor
  close(pd);
  return EXIT_SUCCESS; 
}
