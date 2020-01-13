#include "pipes.h"
#include "utilerias.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>

// Crear un pipe si no existe con el nombre dado, si existe aún así es exitosa la llamada.
void crearPipeSiNoExiste(const char *nombre) {
  if (access(nombre, F_OK) == -1) {
    // Creación
    if (mkfifo(nombre, 0666) != 0 && errno != EEXIST) {
      fprintf(stderr, "Error en pipe: %s\n", nombre);
      terminarProcesos("No se pudo crear el pipe");
    }
  }
}

int abrirPipeEscritura(const char *nombre) {
  int fd;

  crearPipeSiNoExiste(nombre);

  // Apertura
  if ((fd = open(nombre, O_WRONLY)) < 0) {
    fprintf(stderr, "Error en pipe: %s\n", nombre);
    terminarProcesos("No se pudo abrir el pipe");
  }

  return fd;
}

int abrirPipeLectura(const char *nombre) {
  int fd;

  crearPipeSiNoExiste(nombre);

  // Realizar una espera ocupada para la apertura del pipe
  while ((fd = open(nombre, O_RDONLY)) < 0);

  return fd;
}
