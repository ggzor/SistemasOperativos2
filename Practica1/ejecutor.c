// Para que no se incluya otro main de alguna interfaz
#define _MAIN_DEFINIDO_

#include "despachador.h"
#include "listas.h"
#include "planeador.h"
#include "utilerias.h"

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#define SALIDA "salida.txt"

int main() {
  int fd;
  Nodo primero;
  Nodo *cabeza = &primero;

  // Llevar a cabo las planeaciones
  int tiempoTotal = operar(cabeza);

  // Avisar al despachador de la finalización
  terminarDespacho();

  // Ignorar primer nodo centinela
  cabeza = cabeza->siguiente;

  if ((fd = creat(SALIDA, 0666)) < 0)
    exitError("No se pudo crear el archivo de salida.");

  // Información de los procesos
  fprintf(fd, "Tiempo total: %d\n", tiempoTotal);
  fprintf(fd, "PID Tiempo Prioridad Espera Total\n");
  while (cabeza != NULL) {
    fprintf(fd, "\n%3d %6d %9d %6d %5d",
              cabeza->proceso.nombre,
              cabeza->proceso.tiempo,
              cabeza->proceso.prioridad,
              cabeza->proceso.espera,
              cabeza->proceso.tiempo + cabeza->proceso.espera);

    cabeza = cabeza->siguiente;
  }

  close(fd);
}
