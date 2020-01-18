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

int operar(Nodo *lista) {
  int *inicio, *n;
  Proceso proceso;
  int tiempo = 0;
  int terminado = 0;

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

    // Invocar al despachador
    colocar(&proceso, proceso.tiempo);
    tiempo += proceso.tiempo;

    // Verificar el tiempo
    proceso.final = tiempo;

    // Agregar para estadísticas
    agregar(lista, &proceso);
  }

  return tiempo;
}
