#ifndef __PLANEADOR_H__
#define __PLANEADOR_H__

#include "listas.h"
#include "tipos.h"

/**
 * La interfaz mínima que debe implementar cada planeador:
 *  - recibir(proceso):
 *      Recibe un proceso de creación nueva y decide como planificarlo a largo plazo.
 *  - operar(lista):
 *      Realiza la planificacion a corto plazo, una vez que termina un proceso, debe
 *      agregarlo a la lista con las estadísticas correspondientes.
 * 
 * Esta interfaz, permite que los planificadores decidan, de forma independiente,
 * como van a comunicarse internamente y que recursos van a utilizar.
 * 
 * Notas:
 *  - Cada función se llamará en procesos separados.
 **/

void recibir(Proceso *proceso);
void operar(Nodo *lista);

#endif // __PLANEADOR_H__
