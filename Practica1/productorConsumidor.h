#ifndef __PRODUCTOR_CONSUMIDOR__
#define __PRODUCTOR_CONSUMIDOR__

// La definición es externa para que se pueda redefinir
struct Memoria;

// La memoria compartida creada automaticamente
Memoria *memoria;

// Productor
#define producir(code) {iniciarProduccion(); code; terminarProduccion();}
void iniciarProduccion();
void terminarProduccion();
// Este método debe ser llamado cuando se completa la producción
void completarProduccion();

// Consumidor
/** 
 * Se debe declarar una variable del tipo entero siempre que se invoque la macro.
 * Esta variable se pondrá en 1 después de que algún otro proceso haya llamado
 * completarProduccion().
 **/
#define consumir(code) {terminado = iniciarConsumo(); code; terminarConsumo();}
// Se debe devolver 1 cuando ya se haya llamado completarProduccion().
int iniciarConsumo();
void terminarConsumo();

#endif // __PRODUCTOR_CONSUMIDOR__
