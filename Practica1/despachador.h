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

// Define las banderas que puede colocar el planificador para la administracion de memoria
// del proceso
typedef enum { Primera = 1, Normal = 2, Final = 4 } Colocacion;

int colocar(Proceso *proceso, int tiempo, Colocacion colocacion);
void terminarDespacho();

#endif // __DESPACHADOR_H__
