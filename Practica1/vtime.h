#ifndef __VTIME_H__
#define __VTIME_H__

/**
 * Este encabezado prové de una implementación de tiempo virtualizable multiprocesos
 **/

// La cantidad máxima de procesos que van a interactuar con el tiempo virtual
#define MAX_ALARMAS 3

// Dormir la cantidad de tiempo especificada
void vsleep(int s);

// Obtener tiempo global actual
int vtime();

/** 
 * Configurar para usar tiempo real en lugar de virtual.
 *   - La función vsleep(s) llamará a la función sleep(s) de <unistd.h>
 *   - La función vtime() llamará a la función time() de <time.h>
 *   - La función avanzar(s) llamará a la función sleep(s) de <unistd.h>
 * 
 * Esta función debe llamarse antes de que se llamen cualquiera de las otras funciones,
 * para asegurarse del correcto funcionamiento de las operaciones.
 **/
void usarTiempoReal(int tiempoRealActivo);

/**
 * Avanza el tiempo por la cantidad de segundos especificada si se está en modo virtual,
 * de lo contrario, llama a sleep por el tiempo especificado.
 **/
void avanzarTiempo(int s);

#endif // __VTIME_H__
