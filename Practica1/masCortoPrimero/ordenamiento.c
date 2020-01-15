#include "ordenamiento.h"

/* Comparación de proceso por tiempo de ejecución */
int comparacionProcesos(Proceso *p1, Proceso *p2){
  return p1->tiempo - p2->tiempo;
}
