#include "despachador.h"
#include "despachador-comun.h"
#include "pipes.h"
#include "vtime.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Asegurar que hay un solo main
#ifndef _MAIN_DEFINIDO_
#define _MAIN_DEFINIDO_

int main() {
  int tiempo;
  int recepcion, notificacion;
  Proceso proceso;

  // Abrir canales de comunicación
  recepcion = abrirPipeLectura(DESPACHADOR_RECEPCION);
  notificacion = abrirPipeEscritura(DESPACHADOR_NOTIFICACION);

  // Ciclo principal del despachador
  while (1) {
    // Recibir un proceso
    if (read(recepcion, &tiempo, sizeof(int)) <= 0) {
      // Si no se puede leer, quiere decir que ya terminó.
      break;
    }

    read(recepcion, &proceso, sizeof(Proceso));

    // Ejecutar por un tiempo dado
    printf("Ejecutando %d por %ds.\n", proceso.nombre, tiempo);
    tiempo = avanzarTiempo(tiempo);

    // Notificar finalización de ejecución
    write(notificacion, &tiempo, sizeof(int));
  }
}

#endif