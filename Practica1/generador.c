#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int i, cantidad, semilla;
  int duracion, prioridad;
  
  if (argc < 2) {
    printf("Uso: ./generador <cantidad> [semilla=0]\n"
           "  Genera <cantidad> procesos utilizando la [semilla] dada (0 por defecto).\n"
           "  Los resultados se ponen en la salida estándar.\n");
    exit(-1);
  }

  // Procesamiento de parámetros
  cantidad = atoi(argv[1]);
  semilla = argc >= 3 ? atoi(argv[2]) : 0;

  if (cantidad <= 0) {
    printf("ERROR: La cantidad debe ser un entero mayor que cero.\n");
    exit(-1);
  }

  if (semilla < 0) {
    printf("ERROR: La semilla debe ser un entero mayor o igual que cero.\n");
    exit(-1);
  }

  // Generación de números
  srand(semilla);

  for (i = 1; i <= cantidad; i++) {
    // El mínimo se suma y luego se resta para el residuo
    // Se suma 1 porque el rango es inclusivo
    duracion  = 5 + (rand() % (200 - 5 + 1));
    prioridad = 1 + (rand() % (5   - 1 + 1));

    // Poner saltos de línea a partir del segundo
    if (i >= 2)
      printf("\n");

    printf("%d %d %d", i, duracion, prioridad);
  }
}
