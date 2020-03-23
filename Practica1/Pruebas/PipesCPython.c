#include "../memoria.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#define pipeRecepcion "/tmp/pipeEnvioMemoria"
#define pipeEnvio     "/tmp/pipeRecepcionMemoria"

int main() {
  unlink(pipeEnvio);
  unlink(pipeRecepcion);
  mkfifo(pipeEnvio, 0666);
  mkfifo(pipeRecepcion, 0666);

  printf("Pipes creados, enviando primer mensaje\n");
  alojar(0, 5);
  alojar(1, 3);

  acceder(0, 0);
  acceder(0, 2);

  acceder(1, 2);
  acceder(1, 0);

  desalojar(0, 5);
  desalojar(1, 3);

  terminarMemoria();
}
