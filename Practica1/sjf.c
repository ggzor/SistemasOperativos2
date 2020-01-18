// Propias
#include "planificador.h"
#include "semaforos.h"
#include "pipes.h"
#include "utilerias.h"
#include "productorConsumidor.h"
#include "tipos.h"
#include "ordenamiento.h"
#include "despachador.h"

// No Estandar
#include <unistd.h>
#include <sys/sem.h>

// Estandar
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACIDAD 10 // Dimension de la memoria compartida
#define QUANTUM 5

typedef struct Memoria {
  Proceso procesos[CAPACIDAD];
  int n ;
}Memoria;

#define TAMANO sizeof(Memoria)

  /**
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

// int inicializado = 0, fdLector = 0, semaforosMemoria = 0;

void _insercionOrdenada(Proceso *proceso, Proceso *array, int indiceSuperior){
  int i = indiceSuperior;
  // Asignacion de lugar a nuevo proceso. Ordenado de Mayor a Menor
  while(i > 0 &&  comparacionProcesos(&array[i-1], proceso) <= 0 )
    i--;

  // Desplazamiento a la derecha de los valores menores 
  for(int j=indiceSuperior-1; j>=i; j--)
    array[j+1] = array[j];

  memcpy(&array[i], proceso, sizeof(Proceso)); 
}

void recibir(Proceso *proceso){
  int *n;
  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      n = &memoria->n;

      printf("Insercion: %d\n", *n);
      _insercionOrdenada(proceso, memoria->procesos, *n);
      printf("Fin");
      (*n)++;
    });
  }
}

int operar(Nodo *lista){
  Proceso *proceso;
  int terminado = 0;
  int tiempo = 0;
  int *n = NULL ;

  while (!terminado) {
    // Adquirir el último proceso
    consumir({
      n = &memoria->n;
      if(!terminado){
        int numeroProcesos = --(*n); // Numero de slots ocupados(Ya se resto el que se va sacar)
        memcpy(&proceso, &memoria->procesos[numeroProcesos], sizeof(Proceso));
      }
    }); 

    // Colocar en el despachador
    colocar(proceso, proceso->tiempo);
    tiempo += proceso->tiempo;

    // Verificar el tiempo
    proceso->final = tiempo;

    // Agregar para estadísticas
    agregar(lista, proceso);

   }

   return tiempo;
}