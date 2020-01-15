#ifndef __ORDENAIENTO_H__
#define __ORDENAIENTO_H__
#include "tipos.h"

/**
 *  Cabecera que contiene funciones personalizables para definir el ordenamiento de los procesos en un planificador
 *    comparacionProcesos. Función que define la operación de comparación entre dos procesos. 
 *    Retorno.
 *      < 0. El 1er proceso es menor al segundo proceso
 *      0. Los procesos son iguales
 *      > 0. El 1er proceso es mayor al segundo proceso 
 **/

int comparacionProcesos(Proceso *p1, Proceso *p2);

#endif