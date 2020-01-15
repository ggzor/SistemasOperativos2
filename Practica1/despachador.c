#include "despachador.h"
#include "despachador-comun.h"
#include "pipes.h"

#include <unistd.h>
#include <stdio.h>

int abierto = 0, recepcion, notificacion;
void colocar(Proceso *proceso, int tiempo) {
  if (!abierto) {
    // Abrir canales de comunicación
    recepcion = abrirPipeEscritura(DESPACHADOR_RECEPCION);
    notificacion = abrirPipeLectura(DESPACHADOR_NOTIFICACION);
    abierto = 1;
  }

  // Enviar proceso
  write(recepcion, &tiempo, sizeof(int));
  write(recepcion, proceso, sizeof(Proceso));

  // Esperar termino de ejecución
  read(notificacion, &tiempo, sizeof(int));
}

void terminarDespacho() {
  // Con cerrar los pipes se puede terminar el despacho
  if (abierto) {
    close(recepcion);
    close(notificacion);
    abierto = 0;
  }
}
