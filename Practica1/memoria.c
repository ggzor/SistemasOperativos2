#include "memoria.h"
#include "pipes.h"

#include <unistd.h>

#define TERMINAR  -1
#define ALOJAR     0
#define ACCEDER    1
#define DESALOJAR  2

// Los pipes se definen alrevés para comunicarse apropiadamente
#define pipeRecepcion "/tmp/pipeEnvioMemoria"
#define pipeEnvio     "/tmp/pipeRecepcionMemoria"

int abierto = 0;
int recepcion, envio;

void inicializar() {
  if (!abierto) {
    recepcion = abrirPipeLectura(pipeRecepcion);
    envio = abrirPipeEscritura(pipeEnvio);

    abierto = 1;
  }
}

void enviarMensaje(int tipo, int pid, int pag) {
  int buffer[3] = { tipo, pid, pag };

  inicializar();
  write(envio, buffer, 3 * sizeof(int));

  // Esperar la ejecución de la llamada
  read(recepcion, &pid, sizeof(int));
}

void alojar(int pid, int cantidadPaginas) {
  enviarMensaje(ALOJAR, pid, cantidadPaginas);
}

void acceder(int pid, int numeroPagina) {
  enviarMensaje(ACCEDER, pid, numeroPagina);
}

void desalojar(int pid, int cantidadPaginas) {
  enviarMensaje(DESALOJAR, pid, cantidadPaginas);
}

void terminarMemoria() {
  // Notificar terminación del manejo de memoria
  enviarMensaje(TERMINAR, 0, 0);

  if (abierto) {
    close(recepcion);
    close(envio);
  }
}

