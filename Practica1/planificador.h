#ifndef __PLANIFICADOR_H__
#define __PLANIFICADOR_H__

#include "listas.h"
#include "tipos.h"

/**
 * La interfaz mínima que debe implementar cada planificador:
 *  - recibir(proceso):
 *      Recibe un proceso de creación nueva y decide como planificarlo a largo plazo.
 *      Si se recibe NULL, quiere decir que no habrá más procesos.
 *  - operar(lista):
 *      Realiza la planificacion a corto plazo, una vez que termina un proceso, debe
 *      agregarlo a la lista con las estadísticas correspondientes. Devuelve el tiempo
 *      que tardó en ejecutar todos los procesos.
 * 
 * Esta interfaz, permite que los planificadores decidan, de forma independiente,
 * como van a comunicarse internamente y que recursos van a utilizar.
 * 
 * Notas:
 *  - Cada función se llamará en procesos separados.
 **/

void recibir(Proceso *proceso);
int operar(Nodo *lista);

#endif // __PLANIFICADOR_H__
