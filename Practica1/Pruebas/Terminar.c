#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc == 1) {
      system("echo $HOME");

    //if (fork() == 0)
      system("./Terminar 100 &");
    
    //if (fork() == 0)
      system("./Terminar 5 &");
  }
  
  sleep(argc > 1 ? atoi(argv[1]) : 200);
  kill(0, SIGKILL);

  return 0;
}