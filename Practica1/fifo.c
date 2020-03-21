#include "despachador.h"
#include "planificador.h"
#include "productorConsumidor.h"

#include <string.h>

#define CAPACIDAD 10

typedef struct Memoria {
  int inicio, fin;
  int n;
  Proceso procesos[CAPACIDAD];
} Memoria;

#define TAMANO sizeof(Memoria)

void recibir(Proceso *proceso) {
  int *fin, *n;

  inicializarProductor(CAPACIDAD, TAMANO);
  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      fin = &memoria->fin;
      n = &memoria->n;

      // Copiar proceso a la última dirección disponible
      memcpy(&memoria->procesos[*fin], proceso, sizeof(Proceso));

      // Incrementar variables de la cola circular
      *fin = (*fin + 1) % CAPACIDAD;
      (*n)++;
    });
  }
}

void operar(Nodo *lista) {
  int *inicio, *n;
  Proceso proceso;
  int terminado = 0;

  inicializarConsumidor(CAPACIDAD, TAMANO);
  while (!terminado) {
    // Adquirir el último proceso
    consumir({
      if (!terminado) {
        inicio = &memoria->inicio;
        n = &memoria->n;

        memcpy(&proceso, &memoria->procesos[*inicio], sizeof(Proceso));

        // Incrementar variables de la cola circular
        *inicio = (*inicio + 1) % CAPACIDAD;
        (*n)--;
      } else {
        break;
      }
    });

    // Despachar proceso
    proceso.final = colocar(&proceso, proceso.tiempo, Primera | Normal | Final);

    // Agregar para estadísticas
    agregar(lista, &proceso);
  }
}
