#include "despachador.h"
#include "despachador-comun.h"
#include "memoria.h"
#include "pipes.h"

#include <unistd.h>
#include <stdio.h>

int abierto = 0, recepcion, notificacion;
int colocar(Proceso *proceso, int tiempo, Colocacion colocacion) {
  int i, pagina;
  if (!abierto) {
    // Abrir canales de comunicación
    recepcion = abrirPipeEscritura(DESPACHADOR_RECEPCION);
    notificacion = abrirPipeLectura(DESPACHADOR_NOTIFICACION);
    abierto = 1;
  }

  // Interactuar con el manejador de memoria
  if (colocacion & Primera) {
    alojar(proceso->nombre, proceso->cantidadPag);
  }

  if (colocacion & Normal) {
    for (i = 0; i < VENTANA; i++) {
      // Obtener la página siguiente a acceder
      pagina = (proceso->conteo - 1 + i + CADENA_REF_LEN) % CADENA_REF_LEN;
      acceder(proceso->nombre, pagina);
    }
  }
  
  if (colocacion & Final) {
    desalojar(proceso->nombre, proceso->cantidadPag);
  }

  // Enviar proceso
  write(recepcion, &tiempo, sizeof(int));
  write(recepcion, proceso, sizeof(Proceso));

  // Esperar termino de ejecución
  read(notificacion, &tiempo, sizeof(int));

  return tiempo;
}

void terminarDespacho() {
  // Con cerrar los pipes se puede terminar el despacho
  if (abierto) {
    close(recepcion);
    close(notificacion);
    abierto = 0;
  }

  terminarMemoria();
}
