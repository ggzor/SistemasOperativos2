#ifndef __PIPES_H__
#define __PIPES_H__

/** 
 * Abre un pipe nombrado para escribir.
 * 
 * Notas:
 *   - Si el pipe no existe lo crea, de lo contrario solo lo abre para escritura.
 **/
int abrirPipeEscritura(const char *nombre);

/** 
 * Abre un pipe nombrado para leer.
 * 
 * Notas:
 *   - Si el pipe no existe lo crea, de lo contrario solo lo abre para lectura.
 **/
int abrirPipeLectura(const char *nombre);

#endif // __PIPES_H__
