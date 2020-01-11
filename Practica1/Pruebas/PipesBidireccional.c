#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Probar si se puede escribir en un sólo
 * pipe en ambas direcciones.
 * 
 * Conclusiones: 
 *  - No se puede en instantes de tiempos cortos.
 **/

int main() {
  int fd[2];
  char c = 1;
  pipe(fd);

  if (fork() == 0) {
    write(fd[1], &c, 1);
    sleep(1);
    read(fd[0], &c, 1);
    printf("P1: %d\n", c);
  } else {
    read(fd[0], &c, 1);
    write(fd[1], &c, 1);
    printf("P2: %d\n", c);
  }
}