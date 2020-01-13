#include "listas.h"

#include <stdlib.h>
#include <string.h>

Nodo *crear() {
  Nodo *nodo = malloc(sizeof(Nodo));
  return nodo;
}

Nodo *agregar(Nodo *lista, Proceso *proceso) {
  Nodo *nuevo = crear(), *ultimo;
  memcpy(&nuevo->proceso, proceso, sizeof(Proceso));

  if (lista == NULL)
    return nuevo;

  ultimo = lista;
  // Buscar final de la lista
  while (ultimo->siguiente != NULL)
    ultimo = ultimo->siguiente;
  ultimo->siguiente = nuevo;
  
  return lista;
}
