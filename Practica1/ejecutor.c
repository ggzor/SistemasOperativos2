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
  int fd, ejecucion, espera, total;
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
  dprintf(fd, "Tiempo total: %d\n", tiempoTotal);
  dprintf(fd, "PID Tiempo Prioridad Espera Total\n");
  while (cabeza != NULL) { 
    ejecucion = cabeza->proceso.tiempo;
    total = cabeza->proceso.final - cabeza->proceso.inicio;
    espera = total - ejecucion;

    dprintf(fd, "\n%3d %6d %9d %6d %5d",
              cabeza->proceso.nombre,
              cabeza->proceso.tiempo,
              cabeza->proceso.prioridad,
              espera,
              total);

    cabeza = cabeza->siguiente;
  }

  close(fd);
}
