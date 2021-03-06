// Para que no se incluya otro main de alguna interfaz
#define _MAIN_DEFINIDO_

#include "despachador.h"
#include "listas.h"
#include "planificador.h"
#include "utilerias.h"

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define NPRIORIDADES 5

// Variables para estadísticas
// Se pusieron aquí las variables para que se inicialicen en 0.
int cantidadProcesos;
int cantidadProcesosPorPrioridad[NPRIORIDADES];

int esperaTotal;
int esperaPorPrioridad[NPRIORIDADES];

int esperaPorProceso;
int espera;
int i;

int inicio = INT_MAX, final = 0;

int main(int argc, char **argv) {
  int fd, ejecucion, total;
  Nodo primero;
  Nodo *auxiliar;
  Nodo *cabeza;

  if (argc < 2) {
    terminarProcesos(
      "No se proporcionó el nombre del archivo de salida");
  }

  if (argc >= 3)
    srand(atoi(argv[2]));

  cabeza = &primero;

  // Llevar a cabo las planeaciones
  operar(cabeza);

  // Avisar al despachador de la finalización
  terminarDespacho();

  // Ignorar primer nodo centinela
  cabeza = cabeza->siguiente;
  auxiliar = cabeza;

  if ((fd = creat(argv[1], 0666)) < 0)
    exitError("No se pudo crear el archivo de salida.");

  // Calculo de estadísticas avanzadas
  while (auxiliar != NULL) {
    cantidadProcesos++;
    cantidadProcesosPorPrioridad[auxiliar->proceso.prioridad - 1]++;

    espera = auxiliar->proceso.final - auxiliar->proceso.inicio - 
              auxiliar->proceso.tiempo;
    esperaTotal += espera;
    esperaPorPrioridad[auxiliar->proceso.prioridad - 1] += espera;

    // Obtener el tiempo de inicio del primer proceso
    if (auxiliar->proceso.inicio < inicio)
      inicio = auxiliar->proceso.inicio;

    // Obtener el tiempo final del último proceso
    if (auxiliar->proceso.final > final)
      final = auxiliar->proceso.final;

    auxiliar = auxiliar->siguiente;
  }

  // Información de los procesos
  dprintf(fd, "Cantidad procesos: %d\n", cantidadProcesos);
  dprintf(fd, "Tiempo total: %d\n", final - inicio);
  dprintf(fd, "Espera total: %d\n", esperaTotal);
  if (cantidadProcesos > 0)
    dprintf(fd, "Espera promedio: %.2f\n", 1.0f * esperaTotal / cantidadProcesos);

  dprintf(fd, "\nEstadísticas por prioridad:\n");
  dprintf(fd, "Prioridad   Procesos   Espera-Promedio   Espera-Total\n");
  for (i = 0; i < NPRIORIDADES; i++) {
    dprintf(fd, "%5d       %5d      %10.2f     %12d\n", 
      i + 1,
      cantidadProcesosPorPrioridad[i],
      cantidadProcesosPorPrioridad[i] > 0
        ? (1.0f * esperaPorPrioridad[i] / cantidadProcesosPorPrioridad[i])
        : 0,
      esperaPorPrioridad[i]);
  /*
  */
  }
  dprintf(fd, "\n");
  dprintf(fd, "PID Tiempo Prioridad Espera Total");

  while (cabeza != NULL) { 
    ejecucion = cabeza->proceso.tiempo;
    total = cabeza->proceso.final - cabeza->proceso.inicio;
    espera = total - ejecucion;

    dprintf(fd, "\n%3d %6d %5d     %6d %5d",
              cabeza->proceso.nombre,
              ejecucion,
              cabeza->proceso.prioridad,
              espera,
              total);

    cabeza = cabeza->siguiente;
  }

  close(fd);
  return 0;
}
