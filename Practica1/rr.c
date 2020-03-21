// Propias
#include "planificador.h"
#include "semaforos.h"
#include "pipes.h"
#include "utilerias.h"
#include "productorConsumidor.h"
#include "tipos.h"
#include "despachador.h"

#include <unistd.h>
#include <sys/sem.h>
#include <sys/sem.h>
#include <sys/shm.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACIDAD 5
#define QUANTUM 5
#define MUTEX    0
#define LIBRES   1
#define OCUPADOS 2
#define KEY_SEMAFORO 0x12
#define KEY_MEMORIA 0X13

#define INTERACTIVO
typedef struct ProcesoE{
  Proceso proceso;
  int faltante;
}ProcesoE;

typedef struct Memoria {
  ProcesoE procesos[CAPACIDAD];
  int n;
  int i;
}Memoria;

#define TAMANO sizeof(Memoria)

  /**
   * Algoritmo de planificación: Round Robin.
   * 
   * Descripción del algoritmo.
   *        - Los procesos seran ordenados en una cola de procesos entrantes
   *        - El QUANTUM es un valor constante para todos los procesos con un valor de 5 
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
   * _compactar. Compacta los procesos en memoria compartida despues de la eliminación de un proceso
  **/

void _insercion(Proceso *proceso, ProcesoE *array, int n){
  ProcesoE encapsulado;
  memcpy(&encapsulado.proceso, proceso, sizeof(Proceso));
  encapsulado.faltante = encapsulado.proceso.tiempo;
  memcpy(&array[n], &encapsulado, sizeof(ProcesoE));
}

void _compactar(ProcesoE *array, int i, int total){
    // Compactacion de memoria del lado derecho, i es el indice del hueco
    for(int j=i+1; j<total; j++){
      array[j-1] = array[j];
    }
}

void recibir(Proceso *proceso){
  int *n ;

  inicializarProductor(CAPACIDAD, TAMANO);

  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      n = &memoria->n;
      _insercion(proceso, memoria->procesos, *n);
      (*n)++; // Slots ocupados(Contando el que va entrar)
    });
  }
}

void operar(Nodo *lista){
  ProcesoE procesoE;
  int tiempo = 0;
  int *n, *i;
  int terminado = 0;
  int contadorFinalizados = 0;
  int banderaTerminar = 0;
  int esPrimeraVez = 1;
  int rafagaCPU = QUANTUM;
  typedef enum { Despachar, Finalizar } Accion;
  Accion accion;

  // Apertura del semáforo y memoria
  inicializarConsumidor(CAPACIDAD, TAMANO);

  n = &memoria->n;
  i = &memoria->i;
  
  while(!terminado) {

    terminado = decrementarOcupados(); 

    if(!terminado){
      accederMemoriaCompartida({
      // Verificacion de finalizacion de proceso
      memcpy(&procesoE, &memoria->procesos[*i], sizeof(ProcesoE));

        if(memoria->procesos[*i].faltante <= QUANTUM){
          // Verificar el tiempo
          rafagaCPU = procesoE.faltante;

          // Verificacion de compactacion
          if( *i == (*n)-1)
            *i = 0;
          else
            _compactar(memoria->procesos, *i, *n);

          // Actualizar numero de procesos
          (*n)--;
          incrementarLibres();
          accion = Finalizar;
        }else{
          incrementarOcupados();

          // Ajustes de tiempo de proceso
          rafagaCPU = QUANTUM;
          memoria->procesos[*i].faltante -= rafagaCPU;

          // Ajuste de indice
          *i = ((*i)+1)%(*n);
          accion = Despachar;
        }
      });
    }else if(*n == 1){
      // Ultimo proceso en memoria compartida
      memcpy(&procesoE, &memoria->procesos[0], sizeof(ProcesoE));
      rafagaCPU = procesoE.faltante;
      accion = Finalizar;
    }else{
      break;
    }

    switch (accion){
      case Despachar:
        colocar(&procesoE.proceso, rafagaCPU, Normal | (esPrimeraVez ? Primera : 0));
        esPrimeraVez = 0;
      break;
      case Finalizar:
        procesoE.proceso.final = colocar(&procesoE.proceso, rafagaCPU, Normal | Finalizar);
        agregar(lista, &procesoE.proceso);
      break;
    }
  } 

}

