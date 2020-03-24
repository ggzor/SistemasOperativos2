#include "despachador.h"
#include "planificador.h"
#include "productorConsumidor.h"

#include <stdio.h>
#include <string.h>

#define CAPACIDAD 25

typedef struct {
  int restante;
  Proceso proceso;
} PCB;

typedef struct Memoria {
  int n;
  PCB procesos[CAPACIDAD];
} Memoria;

#define TAMANO sizeof(Memoria)

void recibir(Proceso *proceso) {
  int i;
  int *n;
  PCB *procesos;

  inicializarProductor(CAPACIDAD, TAMANO);
  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      n = &memoria->n;
      procesos = memoria->procesos;

      // Insertar nuevo proceso
      for (i = *n - 1; i >= 0 && procesos[i].restante > proceso->tiempo; i--)
        procesos[i + 1] = procesos[i];
      i++;

      memcpy(&procesos[i].proceso, proceso, sizeof(Proceso));
      procesos[i].restante = proceso->tiempo;

      // Incrementar cantidad de espacios ocupados
      (*n)++;
    });
  }
}


void operar(Nodo *lista) {
  int *n;
  int i, tiempoDespacho;

  typedef enum { Despachar, Finalizar } Accion;
  Accion accion;

  Proceso proceso;
  PCB *procesos;

  int terminado = 0;

  inicializarConsumidor(CAPACIDAD, TAMANO);
  while (!terminado) {
    terminado = decrementarOcupados();

    if (!terminado) {
      accederMemoriaCompartida({
        n = &memoria->n;
        procesos = memoria->procesos;

        // Copiar proceso
        procesos[0].proceso.conteo += 1;
        memcpy(&proceso, &procesos[0].proceso, sizeof(Proceso));
        // Solo se asignará un segundo de despacho ya que no es el último
        tiempoDespacho = 1;

        // Si el proceso ya está por terminar
        if (procesos[0].restante == 1) {
          // Compactar
          for (i = 1; i < *n; i++)
            procesos[i - 1] = procesos[i];

          // Notificar incremento de espacios libres
          (*n)--;
          incrementarLibres();

          // La acción que se debe tomar es terminar el proceso
          // después de su despacho
          accion = Finalizar;
        } else {
          procesos[0].restante -= 1;
          incrementarOcupados();

          accion = Despachar;
        }
      });
    } else if (memoria->n == 1) {
      // Copiar último proceso para ser despachado
      memoria->procesos[0].proceso.conteo += 1;
      memcpy(&proceso, &memoria->procesos[0].proceso, sizeof(Proceso));
      tiempoDespacho = memoria->procesos[0].restante;

      accion = Finalizar;
    } else {
      // Terminar planificación
      break;
    }

    switch (accion)
    {
      case Despachar:
        colocar(&proceso, 1, Normal | (proceso.conteo == 1 ? Primera : 0));
        break;
      case Finalizar:
        proceso.final = colocar(&proceso, tiempoDespacho, Normal | Final | (proceso.conteo == 1 ? Primera : 0));
        agregar(lista, &proceso);
        break;
    }
  }
}
