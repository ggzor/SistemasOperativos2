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

#define CAPACIDAD 10
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

 extern int semaforosMemoria;

void _aperturaMemoriaE(int tamano) {
  int shmId;

  // Apertura de memoria compartida
  if ((shmId = shmget(KEY_MEMORIA, tamano, 0666 | IPC_CREAT)) < 0)
    terminarProcesos("Error en la creación de la memoria compartida");

  // Enlace con memoria compartida
  if ((memoria = (struct Memoria *)shmat(shmId, 0, 0)) < 0)
    terminarProcesos("Error al enlazar la memoria compartida");
}

int _verificarFinal(const char *semaforo){
  int banderaFinal = 0;
  if(errno == EINVAL){
    banderaFinal = 1;
  }else {
    char mensajeError[50];
    sprintf(mensajeError,"Error en el semaforo de memoria: %s", semaforo);
    exitError(mensajeError);
  }
  return banderaFinal;
}

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
      procesos[i].numeroVidas = procesos[i].proceso.prioridad * 2;
  }
}

int _ejecutarProceso(Nodo *lista, ProcesoE *siguiente, int *tiempo, int *rafagaCPU){
  int banderaFinal = 0;
  int *i = &memoria->i;
  int *n = &memoria->n;
  ProcesoE aux;
  #ifdef INTERACTIVO
  for(int i=0; i<*n ;i++){
    printf("ID: %d - Faltante: %d\n", memoria->procesos[i].proceso.nombre, memoria->procesos[i].faltante);
  }
  getchar();
  #endif

  // Verificacion de finalizacion de proceso
  if(siguiente->faltante <= QUANTUM){
    memcpy(&aux, siguiente, sizeof(ProcesoE));
    #ifdef INTERACTIVO
    printf("Escribiendo ID: %d - Faltante: %d - indice: %d - total: %d\n", aux.proceso.nombre, aux.faltante, *i, *n);
    getchar();
    #endif

    // Verificar el tiempo
    *rafagaCPU = aux.faltante;
    (*tiempo) += *rafagaCPU;
    aux.proceso.final = *tiempo;

    // Agregar para estadísticas
    agregar(lista, &aux.proceso);

    // Verificacion de compactacion
    if( *i == (*n)-1)
      *i = 0;
    else{
      _compactar(memoria->procesos, *i, *n);
    }

    // Actualizar numero de procesos
    (*n)--;
    if(semIncrementar(semaforosMemoria, LIBRES) == -1)
      banderaFinal = _verificarFinal("LIBRES");

  }else{
    semIncrementar(semaforosMemoria, OCUPADOS);

    *rafagaCPU = QUANTUM;
    siguiente->faltante -= *rafagaCPU;
    (*tiempo) += *rafagaCPU;

    *i = ((*i)+1)%(*n);
  }

  siguiente->numeroVidas--;

  return banderaFinal;
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
  int banderaFinal = 0 ;
  int contadorFinalizados = 0;
  int rafagaCPU = QUANTUM;
  ProcesoE *siguiente = NULL;
  ProcesoE procesoAux;

  // Apertura del semáforo y memoria
  if ((semaforosMemoria = semget(KEY_SEMAFORO, 3, 0666 | IPC_CREAT)) < 0)
    terminarProcesos("Error en la creacion del semáforo del productor-consumidor");
  _aperturaMemoriaE(TAMANO);

  n = &memoria->n;
  i = &memoria->i;
  
  while (1) {
    if(semDecrementar(semaforosMemoria, OCUPADOS) == -1 )
      banderaFinal = _verificarFinal("OCUPADOS");

    // --  REGION CRITICA -- 
    if(semDecrementar(semaforosMemoria, MUTEX) == -1)
      banderaFinal = _verificarFinal("MUTEX");  


    // Obtencion de siguiente y verificacion de fin de epoca
    _siguiente(&siguiente);

    if(siguiente == NULL){
      _restaurarVidas();
      siguiente = &memoria->procesos[0];
      *i = 0;
    }

    banderaFinal = _ejecutarProceso(lista, siguiente, &tiempo, &rafagaCPU);
    memcpy(&procesoAux, siguiente, sizeof(ProcesoE));

    if(banderaFinal && *n <= 0)
        break;

    // -- FIN REGION CRITICA -- 
    if(semIncrementar(semaforosMemoria, MUTEX) == -1)
      banderaFinal = _verificarFinal("MUTEX");  
    
    // Invocar al despachador
    colocar(&procesoAux.proceso, rafagaCPU);
  }

  // Retorna el tiempo total 
  //return tiempo;
}

