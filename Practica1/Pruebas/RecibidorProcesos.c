#include "../pipes.h"
#include "../tipos.h"

#include <stdio.h>
#include <unistd.h>

int main() {
  Proceso proceso;
  int pipe = abrirPipeLectura("/tmp/pipeLector");

  while (read(pipe, &proceso, sizeof(Proceso)) > 0)
    printf("%d\n", proceso.nombre);

  close(pipe);
}