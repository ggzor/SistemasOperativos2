#ifndef __LISTAS_H__
#define __LISTAS_H__

#include "tipos.h"

typedef struct Nodo {
  Proceso proceso;
  struct Nodo *siguiente;
} Nodo;

// Agrega un proceso al final de la lista
Nodo *agregar(Nodo *lista, Proceso *proceso);

#endif // __LISTAS_H__
