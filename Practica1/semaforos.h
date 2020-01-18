#ifndef __SEMAFORO_H__
#define __SEMAFORO_H__ 

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <unistd.h>
#include <errno.h>

/**
 *  Estructuras auxiliares para la manipulación de semaforos:
 *   - struct sembuf decremento. Estructura utilizada con la función semop para el 
 *      decremento del semaforo
 *   - struct sembuf incremento. Estructura utilizada con la función semop para el
 *      incremento del semaforo
 *   - union semun. Uniones necesaria para la manipulacion del semaforo. Versiones
 *      actuales no las agregan a la biblioteca sem.h por default.
 *   - semIncrementar. Funcion que permite incrementar en una unidad el valor del semaforo. El
 *      argumento indicie permite designar el semaforo del conjunto sobre el cual se va tener efecto.
 *      Retorna. Número negativo en caso de haber ocurrido un error
 *   - SemDecremento. La accion contraria a semIncrementar. Decrementa en uno el valor del semaforo en uno
 *      Retorna. Número negativo en caso de haber ocurrido un error
 *   - semEsperarCero: Espera a que el semáforo en el índice especificado llegue a cero.
 *      Retorna. Número negativo en caso de haber ocurrido un error.
 *   - SemValor. Retorna el valor del semaforo seleccionado
 *      Retorna. Número negativo en caso de haber ocurrido un error
 *   - abrirSemaforo. Apertura de semaforo existente de manera obligatoria. Entra en un ciclo
 *      a partir del cual no sale a menos que:
 *        - El semaforo haya sido creado y se haya logrado abrir el semaforo con éxito
 *        - La operación de apertura genero un error distinto al de semaforo no existente
 *      Retorna. Número negativo en caso de haber ocurrido un error. En caso contrario retorna el 
 *      descriptor del pipe
 * 
 *  Nota. Esta cabecera esta desginada para ser utilizada con semaforos de system V
 **/

union semun{
  int val;
  struct semid_ds * buf;
  unsigned short * array;
  #if defined(__linux__)
      struct seminfo * __buf;
  #endif
};

int semIncrementar(int semaforo, int indice);
int semDecrementar(int semaforo, int indice);
int semEsperarCero(int semaforo, int indice);

int semValor(int semaforo, int indice);

int abrirSemaforo(int key);

#endif