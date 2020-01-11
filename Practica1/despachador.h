#ifndef __DESPACHADOR_H__
#define __DESPACHADOR_H__

#include "tipos.h"

/**
 * La interfaz mínima para el despachador de procesos:
 *  - colocar(proceso, tiempo):
 *      Solicita al despachador ejecutar el proceso dado por un intervalo de tiempo dado.
 * Notas:
 *  - La llamada es bloqueante en el proceso que la llama.
 **/
void colocar(Proceso *proceso, int tiempo);

#endif // __DESPACHADOR_H__
