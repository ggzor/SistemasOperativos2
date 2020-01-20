#include "../vtime.h"

#include <unistd.h>
#include <wait.h>

#include <stdio.h>
#include <stdlib.h>

#define VSLEEP(tiempo)                                                         \
  {                                                                            \
    printf("P%d: vsleep(%d)\n", proceso, tiempo);                              \
    vsleep(tiempo);                                                            \
    printf("P%d: vsleep(%d) -> Terminado\n", proceso, tiempo);                 \
  }

#define AVANZAR(tiempo)                                                        \
  {                                                                            \
    printf("P0: avanzar(%d)\n", tiempo);                                       \
    avanzarTiempo(tiempo);                                                     \
  }

#define ESPERAR() { if (!tiempoReal) sleep(2); }

int main(int argc, char **argv) {
  int tiempoReal = 0, proceso = 0;

  if (argc > 1)
    tiempoReal = atoi(argv[1]);
  
  usarTiempoReal(tiempoReal);

  if (fork() == 0) {
    proceso = 1;

    VSLEEP(5);

    _exit(0);
  }

  if (fork() == 0) {
    proceso = 2;

    VSLEEP(2);
    VSLEEP(1);
    VSLEEP(3);

    _exit(0);
  }

  if (fork() == 0) {
    proceso = 3;

    VSLEEP(4);
    VSLEEP(4);

    _exit(0);
  }

  ESPERAR();
  AVANZAR(4);
  ESPERAR();
  AVANZAR(4);

  while (wait(0) != -1);
}