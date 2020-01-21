#include "ordenamiento.h"

/* Comparación de proceso por tiempo de ejecución */
int comparacionProcesosT(Proceso *p1, Proceso *p2){
  return p2->prioridad - p1->prioridad;
}

/* Comparación de procesos por tiempo de ejecución */
int comparacionProcesosP(Proceso *p1, Proceso *p2){
    return p2->prioridad - p1->prioridad;
}
