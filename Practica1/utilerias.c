#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exitError(char *postMessage){
    if(!postMessage || strcmp(postMessage,"") == 0)
        postMessage = "Error.";
    fprintf(stderr,"%s %s", postMessage, strerror(errno));
    exit(EXIT_FAILURE);
}

void terminarProcesos(const char *mensaje) {
  perror(mensaje);
  // Enviar SIGKILL a todos los procesos
  kill(0, SIGKILL);
}