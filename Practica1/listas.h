#ifndef __LISTAS_H__
#define __LISTAS_H__

#include "tipos.h"

typedef struct Nodo {
  Proceso proceso;
  Nodo *siguiente;
} Nodo;

// Agrega un proceso al final de la lista
void agregar(Nodo *lista, Proceso *proceso);

#endif // __LISTAS_H__
