#include "despachador.h"
#include "pipes.h"

#include <unistd.h>
#include <stdio.h>

#define DESPACHADOR_RECEPCION    "/tmp/pipeDespachadorRecepcion"
#define DESPACHADOR_NOTIFICACION "/tmp/pipeDespachadorNotificacion"

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

// Asegurar que hay un solo main
#ifndef _MAIN_DEFINIDO_
#define _MAIN_DEFINIDO_

int main() {
  int tiempo;
  Proceso proceso;

  // Abrir canales de comunicación
  recepcion = abrirPipeLectura(DESPACHADOR_RECEPCION);
  notificacion = abrirPipeEscritura(DESPACHADOR_NOTIFICACION);

  // Ciclo principal del despachador
  while (1) {
    // Recibir un proceso
    if (read(recepcion, &tiempo, sizeof(int)) <= 0) {
      // Si no se puedo leer, quiere decir que ya terminó.
      break;
    }

    read(recepcion, &proceso, sizeof(Proceso));

    // Ejecutar por un tiempo dado
    printf("Ejecutando %d por %ds.\n", proceso.nombre, tiempo);
    sleep(tiempo);

    // Notificar finalización de ejecución
    write(notificacion, &tiempo, sizeof(int));
  }
}

#endif