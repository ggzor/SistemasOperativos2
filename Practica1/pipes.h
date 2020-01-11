#ifndef __PIPES_H__
#define __PIPES_H__

/** 
 * Abre un pipe nombrado para escribir.
 * 
 * Notas:
 *   - Si hay un pipe existente lo borra y lo vuelve a crear.
 **/
int abrirPipeEscritura(const char *nombre);

/** Abre un pipe nombrado para leer.
 * 
 * Notas:
 *   - Si el pipe no existe, se espera de forma ocupada hasta que se cree.
 **/
int abrirPipeLectura(const char *nombre);

#endif // __PIPES_H__
