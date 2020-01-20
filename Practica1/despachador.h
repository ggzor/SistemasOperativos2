#ifndef __DESPACHADOR_H__
#define __DESPACHADOR_H__

#include "tipos.h"

/**
 * La interfaz mínima para el despachador de procesos:
 *  - colocar(proceso, tiempo):
 *      Solicita al despachador ejecutar el proceso dado por un intervalo de tiempo dado.
 *      Devuelve el tiempo actual del procesador.
 *  - terminarDespacho():
 *      Notifica al despachador que no se colocarán más procesos.
 * Notas:
 *  - La llamada es bloqueante en el proceso que la llama.
 **/
int colocar(Proceso *proceso, int tiempo);
void terminarDespacho();

#endif // __DESPACHADOR_H__
