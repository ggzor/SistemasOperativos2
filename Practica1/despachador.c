#include "despachador.h"
#include "pipes.h"

#include <unistd.h>
#include <stdio.h>

#define DESPACHADOR_RECEPCION    "/tmp/pipeDespachadorRecepcion"
#define DESPACHADOR_NOTIFICACION "/tmp/pipeDespachadorNotificacion"

int abierto = 0, recepcion, notificacion;
int colocar(Proceso *proceso, int tiempo) {
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

int main() {
  int tiempo;
  Proceso proceso;

  // Abrir canales de comunicación
  recepcion = abrirPipeLectura(PIPE_DESPACHADOR_RECEPCION);
  notificacion = abrirPipeEscritura(PIPE_DESPACHADOR_NOTIFICACION);

  // Ciclo principal del despachador
  while (1) {
    // Recibir un proceso
    read(recepcion, &tiempo, sizeof(int));
    read(recepcion, &proceso, sizeof(Proceso));

    // Ejecutar por un tiempo dado
    printf("Ejecutando %d por %ds.\n", proceso.nombre, tiempo);
    sleep(tiempo);

    // Notificar finalización de ejecución
    write(notificacion, &tiempo, sizeof(int));
  }
}
