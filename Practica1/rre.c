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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACIDAD 20
#define QUANTUM 5
#define MUTEX    0
#define LIBRES   1
#define OCUPADOS 2
#define KEY_SEMAFORO 0x12
#define KEY_MEMORIA 0X13

typedef struct ProcesoE{
  Proceso proceso;
  int numeroVidas;
  int faltante;
}ProcesoE;

typedef struct Memoria {
  ProcesoE procesos[CAPACIDAD];
  int n;
  int i;
}Memoria;

#define TAMANO sizeof(Memoria)
  /**
   * Algoritmo de planificación: Round Robin Por Epocas.
   * 
   * Descripción del algoritmo.
   *    - Los procesos entraran en una cola de procesos entrantes con un número de vidas igual a 0.
   *    - Los proceos seran despachados conforme a su orden en la cola siempre y cuando tengan vidas por consumir durante esa epoca
   *    - La cantidad de CPU otorgara variara dependiendo de su prioridad
   *    - La cantidad de vidas por epoca variara dependiendo de su prioridad y sera reiniciada cada epoca, es decir, cuando todos los proceso tengan 0 vidas en la epoca
   *      epoca actual
   * 
   * Recibir. Función encarga de recibir Procesos en la parte del Receptor. Los procesos son pasados por el lector de procesos
   *          Tiene como tárea crear la lógica de comunicación con la funcion Operar que es ejecutada en otro proceso.
   * Operar. Función encargada de recibir Procesos en la parte del Ejecutor. Los procesos son mandados por el Receptor a tráves 
   *          de la función recibir.
   *          Aplica las politicas de planificación para simular el algoritmo de planificación
   * _compactar. Compacta los procesos en memoria compartida despues de la eliminación de un proceso
   * _restaurarVidas. Restaura las vidas de cada proceso en memoria compartida. Las politicas de asignación de vidas son proporcionales a su prioridad
   * _insercion. Insecion de un proceso en memoria compartida
   * _siguiente. Recupera el siguiente proceso en ser ejecutado. En caso de no existir ningun proceso con vidas en esa epoca, se retorna NULL 
  **/


void _compactar(ProcesoE *array, int i, int total){
    // Compactacion de memoria del lado derecho, i es el indice del hueco
    for(int j=i+1; j<total; j++){
      array[j-1] = array[j];
    }
}

void _restaurarVidas(){
  ProcesoE *procesos = memoria->procesos;
  int n  = memoria->n;
  for(int i=0; i<n; i++){
      procesos[i].numeroVidas = pow(2, procesos[i].proceso.prioridad - 1) * 2 ; //procesos[i].proceso.prioridad * 2;
  }
}

void _insercion(Proceso *proceso){
  ProcesoE encapsulado;
  int n = memoria->n;
  ProcesoE *array = memoria->procesos;

  memcpy(&encapsulado.proceso, proceso, sizeof(Proceso));
  encapsulado.faltante = encapsulado.proceso.tiempo;
  encapsulado.numeroVidas = 0;
  memcpy(&array[n], &encapsulado, sizeof(ProcesoE));
}

void _siguiente(ProcesoE **siguiente){
    ProcesoE *procesos = memoria->procesos;
    int* n = &memoria->n;
    int *i = &memoria->i;
    int aux = *i;
    int encontrado = 0;
    *siguiente = NULL;
    do{
        if(procesos[*i].numeroVidas > 0){
          *siguiente = &procesos[*i];
        }else{
          *i = ((*i)+1)%(*n);
        }
    }while(aux != *i && *siguiente == NULL);

}

void recibir(Proceso *proceso){
  int *n ;
  inicializarProductor(CAPACIDAD, TAMANO);

  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      n = &memoria->n;
      _insercion(proceso);
      (*n)++; // Slots ocupados(Contando el que va entrar)
    });
  }
}

void operar(Nodo *lista){
  int tiempo = 0;
  int *n, *i;
  int terminado = 0;
  int contadorFinalizados = 0;
  int rafagaCPU = QUANTUM;
  int quantumVariable = 0;
  ProcesoE *siguiente = NULL;
  ProcesoE procesoE;
  typedef enum { Despachar, Finalizar } Accion;
  Accion accion;

  // Apertura del semáforo y memoria
  inicializarConsumidor(CAPACIDAD, TAMANO);

  n = &memoria->n;
  i = &memoria->i;
  
  while (!terminado){
    terminado = decrementarOcupados(); 

    if(!terminado){
      accederMemoriaCompartida({
        // Obtencion de siguiente y verificacion de fin de epoca
        _siguiente(&siguiente);

        if(siguiente == NULL){
          _restaurarVidas();
          *i = 0;
          siguiente = &memoria->procesos[*i];
        }

        memcpy(&procesoE, siguiente, sizeof(ProcesoE));
        quantumVariable = sqrt((6 - siguiente->proceso.prioridad) * 10);

        if(siguiente->faltante <= quantumVariable){
          // Verificar el tiempo
          rafagaCPU = siguiente->faltante;

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

          // Ajuste de tiempo de proceso
          rafagaCPU = quantumVariable;
          siguiente->faltante -= rafagaCPU;
          
          // Ajuste de indice
          *i = ((*i)+1)%(*n);
          accion = Despachar;
        } 

        siguiente->numeroVidas--;
      });
    } else if (*n == 1){
      // Ultimo proceso en memoria compartida
      memcpy(&procesoE, siguiente, sizeof(ProcesoE));
      rafagaCPU = procesoE.faltante;
      accion  = Finalizar;
    } else{
      // Finalizo
      break;
    }

    switch (accion){
      case Despachar:
        colocar(&procesoE.proceso, rafagaCPU);
      break;
      case Finalizar:
        procesoE.proceso.final = colocar(&procesoE.proceso, rafagaCPU);
        agregar(lista, &procesoE.proceso);
      break;
    }
  }
}

