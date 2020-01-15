// Propias 
#include "semaforos.h"

struct sembuf decremento = { 0, -1, SEM_UNDO}; 
struct sembuf incremento = { 0, +1, SEM_UNDO}; 

int semIncrementar(int semaforo, int indice){
    return semop(semaforo, &incremento, 1);
}

int semDecrementar(int semaforo, int indice){
    return semop(semaforo, &decremento, 1);
}

int semValor(int semaforo, int indice){
    union semun arg;
    return semctl(semaforo, indice,  GETVAL, arg); 
}

int abrirSemaforo(int key){
  int fd = 0;

  // Intento de apertura de archivo y espera ocupada en caso de no existir.
  while((fd=semget(key, 3, 0666)) < 0 && fd != ENOENT);

  return fd;
}
