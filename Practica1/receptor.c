// Para que no se incluya otro main de alguna interfaz
#define _MAIN_DEFINIDO_

#include "pipes.h"
#include "planeador.h"
#include "tipos.h"
#include "utilerias.h"

#include <unistd.h>

#include <stdlib.h>

#define RECEPTOR "/tmp/pipeReceptor"

int main() {
  int recepcion;
  Proceso proceso;

  recepcion = abrirPipeLectura(RECEPTOR);

  // Recibir cada proceso de uno en uno
  while (read(recepcion, &proceso, sizeof(proceso)) > 0)
    recibir(&proceso);

  // Indicar que se termino la producción
  recibir(NULL);
  
  return EXIT_SUCCESS;
}
