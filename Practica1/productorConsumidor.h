#ifndef __PRODUCTOR_CONSUMIDOR__
#define __PRODUCTOR_CONSUMIDOR__

// La definición es externa para que se pueda redefinir
struct Memoria;

// La memoria compartida creada automaticamente que se establecerá
// en 0 todos sus campos al solicitarla por primera vez.
extern struct Memoria *memoria;

// Productor
/**
 * Para usar esta macro, se deben definir las macros CAPACIDAD y TAMANO
 * que representan la capacidad de producción y el tamaño de la estructura
 * memoria, respectivamente.
 **/
#define producir(code) {iniciarProduccion(CAPACIDAD, TAMANO); code; terminarProduccion();}
void iniciarProduccion(int capacidad, int tamano);
void terminarProduccion();
// Este método debe ser llamado cuando se completa la producción
void completarProduccion();

// Consumidor
/** 
 * Para usar esta macro, se deben definir las macros CAPACIDAD y TAMANO
 * que representan la capacidad de producción y el tamaño de la estructura
 * memoria, respectivamente.
 *
 * Se debe declarar una variable del tipo entero "terminado" siempre que se invoque la macro.
 * Esta variable se pondrá en 1 después de que algún otro proceso haya llamado
 * completarProduccion().
 **/
#define consumir(code) {terminado = iniciarConsumo(CAPACIDAD, TAMANO); code; terminarConsumo();}
// Se debe devolver 1 cuando ya se haya llamado completarProduccion().
int iniciarConsumo(int capacidad, int tamano);
void terminarConsumo();

// Limpia todos los recursos ocupados por el procesos de consumir y producir. 
// Esta función se ejecutará al terminar la ejecución y al iniciar otra para
// asegurar que los recursos estén en un estado limpio.
void limpiarRecursos();

#endif // __PRODUCTOR_CONSUMIDOR__
