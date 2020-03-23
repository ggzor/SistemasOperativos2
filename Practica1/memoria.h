#ifndef __MEMORIA_H__
#define __MEMORIA_H__

/**
  * Define la interfaz para interactuar con el administrador de
  * memoria
  *
  *  - alojar(pid, cantidadPaginas):
  *      Solicita al manejador de memoria alojar la cantidad de paginas
  *      especificada para el proceso con el pid dado
  *
  *  - acceder(pid, numeroPagina):
  *      Solicita al manejador de memoria acceder a la página dada
  *
  *  - desalojar(pid, cantidadPaginas):
  *      Solicita al manejador de memoria desalojar la cantidad de paginas
  *      especificada
  *
  *  - terminarMemoria():
  *      Informa al manejador de memoria que debe terminar
  **/

void alojar(int pid, int cantidadPaginas);
void acceder(int pid, int numeroPagina);
void desalojar(int pid, int cantidadPaginas);
void terminarMemoria();

#endif
