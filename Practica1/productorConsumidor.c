#include "productorConsumidor.h"
#include "semaforos.h"
#include "utilerias.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define KEY_SEMAFORO 0x12
#define KEY_MEMORIA 0X13

#define MUTEX    0
#define LIBRES   1
#define OCUPADOS 2

/** Funciones auxiliares para la apertura e inicialización de los semaforos comunes entre el ejecutor y el recepetor
 *  _aperturaMemoria. Apertura de memoria compartida entre ambos procesos.
 *  _inicializarSemaforos. Inicialización de los semaforos:
 *    - Mutex. Semaforo binario (Valor inicial 1)
 *    - Slots libres. Semaforo que lleva el contador de los slots libres en memoria compartida (Valor Inicial capacidad)
 *    - Slots ocupados. Semaforo que lleva el contador de los slots ocupados en memoria compartida (Valor Inicial 0)
 **/

int inicializado = 0 ;
int semaforosMemoria = 0;
int fdLector = 0;

struct Memoria *memoria;

void _aperturaMemoria(int tamano) {
  int shmId;

  // Apertura de memoria compartida
  if ((shmId = shmget(KEY_MEMORIA, tamano, 0666 | IPC_CREAT)) < 0)
    terminarProcesos("Error en la creación de la memoria compartida");

  // Enlace con memoria compartida
  if ((memoria = (struct Memoria *)shmat(shmId, 0, 0)) < 0)
    terminarProcesos("Error al enlazar la memoria compartida");
}

void _inicializarSemaforos(int capacidad) {
  union semun arg;

  arg.val = 1;
  semctl(semaforosMemoria, MUTEX, SETVAL, arg); 

  arg.val = capacidad ;
  semctl(semaforosMemoria, LIBRES, SETVAL, arg);

  arg.val = 0;
  semctl(semaforosMemoria, OCUPADOS, SETVAL, arg);
}

void iniciarProduccion(int capacidad, int tamano) {
  if (!inicializado) {
    // Apertura del semáforo
    if ((semaforosMemoria = semget(KEY_SEMAFORO, 3, 0666 | IPC_CREAT)) < 0)
      terminarProcesos("Error en la creacion del semáforo del productor-consumidor");

    _inicializarSemaforos(capacidad);
    _aperturaMemoria(tamano);

    // Inicializar en 0
    memset(memoria, 0, tamano);

    inicializado = 1;
  }
  // Bloquear hasta que haya espacios libres
  semDecrementar(semaforosMemoria, LIBRES);
  semDecrementar(semaforosMemoria, MUTEX);
}

void terminarProduccion() {
  semIncrementar(semaforosMemoria, MUTEX);
  // Incrementar la cantidad de espacios ocupados
  semIncrementar(semaforosMemoria, OCUPADOS);
}

void completarProduccion() {
  // Se espera a que el consumidor termine de consumir todos los procesos
  semEsperarCero(semaforosMemoria, OCUPADOS);

  // Remover semáforos para provocar desbloqueo
  semctl(semaforosMemoria, 0, IPC_RMID);
}

int iniciarConsumo(int capacidad, int tamano) {
  if (!inicializado) {
    semaforosMemoria = abrirSemaforo(KEY_SEMAFORO);
    _aperturaMemoria(tamano);

    inicializado = 1;
  }

  // Bloquear hasta que haya espacios ocupados
  if (semDecrementar(semaforosMemoria, OCUPADOS) < 0) {
    // Terminar cuando no se pueda decrementar
    return 1;
  }
  semDecrementar(semaforosMemoria, MUTEX);

  // Si se pudo decrementar entonces se procede exitosamente
  return 0;
}

void terminarConsumo() {
  semIncrementar(semaforosMemoria, MUTEX);
  // Incrementar la cantidad de espacios libres
  semIncrementar(semaforosMemoria, LIBRES);
}

void limpiarRecursos() {
  if (semctl(semget(KEY_SEMAFORO, 0, 0666), 0, IPC_RMID) >= 0)
    printf("Se borró el semáforo del productor consumidor.\n");

  if (shmctl(shmget(KEY_MEMORIA, 4, 0666), IPC_RMID, 0) >= 0)
    printf("Se borró el segmento de memoria compartida del productor consumidor.\n");
}
