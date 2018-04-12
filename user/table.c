#include "table.h"

philosophers philosopher[16];
int forks[16];
int pid;
extern void main_philosopher();

void main_table(){
  write( STDOUT_FILENO, "Table Created: ", 15 );
  pid = getpid();
  mkfifo(pid, 0);
  popen(pid, 0);
  int consolepipe = pfind(pid, 0);
  pwrite(consolepipe, 666);
  for(int i = 0; i < 16; i++){
    pid_t pid = fork();
    if(pid == 0){
      exec(&main_philosopher);
    }
  }
  while(1){
    for(int i = 0; i < 16; i++){

    }
  }
  exit( EXIT_SUCCESS );
}
