#ifndef __TIPOS_H__
#define __TIPOS_H__

#define CADENA_REF_LEN 20

typedef struct {
    int nombre;
    int tiempo;
    int prioridad;
    int cantidadPag;
    int cadenaReferencias[CADENA_REF_LEN];

    int inicio;
    int final;
 } Proceso;

#endif // __TIPOS_H__
