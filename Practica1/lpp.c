#include "despachador.h"
#include "planificador.h"
#include "productorConsumidor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CAPACIDAD 25
#define QUANTUM 5

typedef struct {
  int restante;
  int boletos;
  float probabilidad;
  Proceso proceso;
} PCB;

typedef struct Memoria {
  int n;
  int totalBoletos;
  PCB procesos[CAPACIDAD];
} Memoria;

#define TAMANO sizeof(Memoria)

void recalcularProbabilidades() {
  int *n = &memoria->n;
  PCB *procesos = memoria->procesos;
  int i;

  for (i = 0; i < *n; i++)
    procesos[i].probabilidad = 1.0f * procesos[i].boletos / memoria->totalBoletos;
}

void recibir(Proceso *proceso) {
  int *n, i;
  PCB *procesos;

  inicializarProductor(CAPACIDAD, TAMANO);
  if (proceso == NULL) {
    completarProduccion();
  } else {
    producir({
      n = &memoria->n;
      procesos = memoria->procesos;

      memcpy(&procesos[*n].proceso, proceso, sizeof(Proceso));

      // Inicializar variables de control
      procesos[*n].restante = proceso->tiempo;
      procesos[*n].boletos = proceso->prioridad;
      memoria->totalBoletos += procesos[*n].boletos;
      // Incrementar cantidad de espacios ocupados
      (*n)++;

      recalcularProbabilidades();

      for (i = 0; i < *n; i++)
        printf("PID %d = %.2f\n", procesos[i].proceso.nombre, procesos[i].probabilidad);
    });
  }
}

void operar(Nodo *lista) {
  int *n;
  int i, j, tiempoDespacho;
  float ganador, suma;

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

        // Selección del ganador
        ganador = 1.0f * rand() / RAND_MAX;
        suma = 0.0f;

        for (i = 0; i < *n && ganador < suma; i++)
          suma += procesos[i].probabilidad;

        // Copiar proceso
        memcpy(&proceso, &procesos[i].proceso, sizeof(Proceso));

        // Si el proceso ya está por terminar
        if (procesos[i].restante <= QUANTUM) {
          // Sólo se debe despachar por su tiempo restante
          tiempoDespacho = procesos[i].restante;
          // Quitar los boletos
          memoria->totalBoletos -= procesos[i].boletos;
          recalcularProbabilidades();

          // Compactar
          for (j = i + 1; j < *n; j++)
            procesos[i - 1] = procesos[i];

          // Notificar incremento de espacios libres
          (*n)--;
          incrementarLibres();

          // La acción que se debe tomar es terminar el proceso
          // después de su despacho
          accion = Finalizar;
        } else {
          procesos[i].restante -= QUANTUM;
          incrementarOcupados();

          // Se despacha por el quantum
          tiempoDespacho = QUANTUM;
          accion = Despachar;
        }
      });
    } else if (memoria->n == 1) {
      // Copiar último proceso para ser despachado
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
        colocar(&proceso, 1);
        break;
      case Finalizar:
        proceso.final = colocar(&proceso, tiempoDespacho);
        agregar(lista, &proceso);
        break;
    }
  }
}
