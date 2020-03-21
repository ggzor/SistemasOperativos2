#include "planificador.h"
#include "semaforos.h"
#include "pipes.h"
#include "utilerias.h"
#include "productorConsumidor.h"
#include "tipos.h"
#include "ordenamiento.h"
#include "despachador.h"

#include <unistd.h>
#include <sys/sem.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACIDAD 15

typedef struct Memoria {
  Proceso procesos[CAPACIDAD];
  int n;
}Memoria;

#define TAMANO sizeof(Memoria)

  /**
   * Algoritmo de planificación Shortest Job First
   * 
   * Descripción del algoritmo.
   *        - Los procesos ingresados seran ordenados en una cola conforme a su tiempo de ejecución total
   *        - El procesamiento va ser por lotes, es decir, no existe un QUANTUM que delimite el tiempo que se le asignara a un proceso. El tiempo de CPU
   *          de cada proceso es equivalente al tiempo de ejecución de dicho proceso.
   * 
   * Funciones para algoritmo de planificación de procesos con politicas de Short Job First.
   * _insercionOrdenda. Funcion encarga de insertar un nuevo proceso en la memoria compartida de manera 
   *                  que se respete el orden. La función encargada de hacer la comparación entre procesos
   *                  se encuentra definida en la cabecera ordenamiento.h
   * Recibir. Función encarga de recibir Procesos en la parte del Receptor. Los procesos son pasados por el lector de procesos
   *          Tiene como tárea crear la lógica de comunicación con la funcion Operar que es ejecutada en otro proceso.
   * Operar. Función encargada de recibir Procesos en la parte del Ejecutor. Los procesos son mandados por el Receptor a tráves 
   *          de la función recibir.
   *          Aplica las politicas de planificación para simular el algoritmo de planificación
  **/


void _insercionOrdenada(Proceso *proceso, Proceso *array, int total){
  int i = total;

  // Asignacion de lugar a nuevo proceso. Ordenado de Mayor a Menor
  while(i > 0 &&  comparacionProcesosP(&array[i-1], proceso) <= 0 )
    i--;

  // Desplazamiento a la derecha de los valores menores 
  for(int j=total-1; j >= i; j--)
    array[j+1] = array[j];

  array[i] = *proceso; 
}

void recibir(Proceso *proceso){
  int *n ;

  inicializarProductor(CAPACIDAD, TAMANO);
  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      n = &memoria->n;
      _insercionOrdenada(proceso, memoria->procesos, *n);
      (*n)++;
      
    });
  }
}

void operar(Nodo *lista) {
  Proceso proceso;
  int terminado = 0;
  int *n;

  inicializarConsumidor(CAPACIDAD, TAMANO);
  while (!terminado) {
    // Adquirir el último proceso
    consumir({
      if (!terminado) {
        n = &memoria->n;
        
        (*n)--;
        memcpy(&proceso, &memoria->procesos[*n], sizeof(Proceso));
      } else {
        break;
      }
    });

    // Verificar el tiempo
    proceso.final = colocar(&proceso, proceso.tiempo, Primera | Normal | Final);

    // Agregar para estadísticas
    agregar(lista, &proceso);
  }
}
