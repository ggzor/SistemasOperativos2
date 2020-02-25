#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int x, y;
  char t[10];
} Data;

#define FORMAT "%d %d %s"
#define COMMAND_FORMAT "python3 pipes.py %2d %2d"
#define READ  0
#define WRITE 1

int main() {
  int size = sizeof(Data);
  Data d   = { 10, 20, "Hello" };
  int cToPython[2], pythonToC[2];
  char command[] = COMMAND_FORMAT;

  pipe(cToPython);
  pipe(pythonToC);

  if (fork() == 0) {
    sprintf(command, COMMAND_FORMAT, cToPython[READ], pythonToC[WRITE]);
    system(command);
    exit(0);
  }

  printf("Initial data: %d %d %s\n", d.x, d.y, d.t);

  // Write size first
  write(cToPython[WRITE], &size, sizeof(int));
  write(cToPython[WRITE], &d, size);

  read(pythonToC[READ], &d, size);

  printf("Final data:   %d %d %s\n", d.x, d.y, d.t);
}
