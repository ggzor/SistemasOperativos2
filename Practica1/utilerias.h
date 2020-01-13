#ifndef __UTILERIAS_H__
#define __UTILERIAS_H__

/** 
 * Termina todos los procesos del grupo de procesos con el
 * mensaje de error dado imprimiendo errno.
 **/
void terminarProcesos(const char *mensaje);

/**
 * Termina el proceso que llama la función imprimiendo errno.
 **/
void exitError(char *postMessage);

#endif // __UTILERIAS_H__
