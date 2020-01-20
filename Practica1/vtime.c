#include "semaforos.h"
#include "utilerias.h"
#include "vtime.h"

#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define KEY_VTIME 0x33
#define KEY_VTIME_SHM 0x34

typedef struct {
  int pid;
  int tiempo;
} Alarma;

typedef struct {
  int n;
  Alarma alarmas[MAX_ALARMAS];
  int tiempoActual;
  int tiempoRealActivo;
} Memoria;

int mutex;
Memoria *memoria;

int inicializado;
void inicializarMemoriaCompartida() {
  int shmid;
  if (!inicializado) {
    // Borrar si existe
    if ((shmid = shmget(KEY_VTIME_SHM, 0, 0666)) != -1)
      shmctl(shmid, IPC_RMID, NULL);

    shmid = shmget(KEY_VTIME_SHM, sizeof(Memoria), 0666 | IPC_CREAT);
    printf("%d\n", sizeof(Memoria));

    if (shmid == -1)
      terminarProcesos("No se pudo abrir la memoria compartida para el tiempo virtual");
    
    memoria = shmat(shmid, 0, 0);

    inicializado = 1;
  }
}

void usarTiempoReal(int tiempoRealActivo) {
  // Inicializar mutex
  mutex = semget(KEY_VTIME, 1, 0666 | IPC_CREAT);
  if (mutex == -1)
    terminarProcesos("No se pudo abrir el semáforo para el tiempo virtual");
  if (semctl(mutex, 0, SETVAL, 1) == 1)
    terminarProcesos("No se pudo inicializar el semáforo del tiempo virtual");

  inicializarMemoriaCompartida();
  memset(memoria, 0, sizeof(Memoria));

  memoria->tiempoRealActivo = tiempoRealActivo;
}

int tiempoRealVerificado = 0;
int tiempoRealActivo;
int estaTiempoRealActivo() {
  inicializarMemoriaCompartida();

  // Verificar que el tiempo real esté activo una sola vez
  if (!tiempoRealVerificado) {
    semDecrementar(mutex, 0);
    {
      tiempoRealActivo = memoria->tiempoRealActivo;
    }
    semIncrementar(mutex, 0);

    tiempoRealVerificado = 1;
  }

  return tiempoRealActivo;
}

int tiempoLocal = 0;
sigset_t mascara;
int enmascaradoIniciado = 0;
void vsleep(int s) {
  if (!enmascaradoIniciado) {
    sigemptyset(&mascara);
    sigaddset(&mascara, SIGUSR1);

    sigprocmask(SIG_BLOCK, &mascara, NULL);

    enmascaradoIniciado = 1;
  }

  int esperar, senalRecibida;
  int *n;

  // Un sleep de 0, regresa inmediatamente
  if (s == 0)
    return;

  // Un sleep negativo es inválido
  if (s < 0)
    terminarProcesos("No se puede dormir un tiempo negativo");

  if (estaTiempoRealActivo()) {
    sleep(s);
  } else {
    esperar = 0;

    // Registrar alarma
    semDecrementar(mutex, 0);
    {
      // Si se despierta en un instante futuro
      if (tiempoLocal + s > memoria->tiempoActual) {
        n = &memoria->n;

        if (*n >= MAX_ALARMAS)
          terminarProcesos("No se pueden registrar más alarmas que el máximo de alarmas");

        memoria->alarmas[*n].pid = getpid();
        memoria->alarmas[*n].tiempo = tiempoLocal + s;

        (*n)++;

        esperar = 1;
      }
    }
    semIncrementar(mutex, 0);

    // Esperar a ser despertado
    if (esperar)
      sigwait(&mascara, &senalRecibida);

    tiempoLocal += s;
  }
}

int vtime() {
  if (estaTiempoRealActivo()) {
    return time(NULL);
  } else {
    return tiempoLocal;
  }
}

void avanzarTiempo(int s) {
  int i, j;
  int *n;

  if (estaTiempoRealActivo()) {
    sleep(s);
  } else {
    semDecrementar(mutex, 0);
    {
      n = &memoria->n;

      memoria->tiempoActual += s;

      for (i = 0; i < *n; i++) {
        if (memoria->alarmas[i].tiempo <= memoria->tiempoActual) {
          printf("Despertando %d pid, (n = %d)\n", memoria->alarmas[i].pid, *n);

          // Despertar proceso
          kill(memoria->alarmas[i].pid, SIGUSR1);

          // Compactar
          for (j = i + 1; j < *n; j++)
            memoria->alarmas[j - 1] = memoria->alarmas[j];

          // Conservar índice en posición
          i--;

          // Decrementar n
          (*n)--;
        }
      }
    }
    semIncrementar(mutex, 0);
  }
}