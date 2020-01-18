// Propias 
#include "semaforos.h"

struct sembuf decremento = { .sem_num = 0, .sem_op = -1, .sem_flg = 0 }; 
struct sembuf incremento = { .sem_num = 0, .sem_op = +1, .sem_flg = 0 }; 
struct sembuf cero       = { .sem_num = 0, .sem_op =  0, .sem_flg = 0 };

int semIncrementar(int semaforo, int indice){
  incremento.sem_num = indice;
  return semop(semaforo, &incremento, 1);
}

int semDecrementar(int semaforo, int indice){
  decremento.sem_num = indice;
  return semop(semaforo, &decremento, 1);
}

int semEsperarCero(int semaforo, int indice) {
  cero.sem_num = indice;
  return semop(semaforo, &cero, 1);
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
