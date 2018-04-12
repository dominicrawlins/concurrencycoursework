#include "philosopher.h"
int pid;
#define left 0
#define right 1
#define table 2

processes process[3];


void main_philosopher(){
  write( STDOUT_FILENO, "Phill Created: ", 15 );
  pid = getpid();
  for(int i = 0; i < 100; i++){
    int pipenumber = pfind(i,0);
    if(pread(pipenumber) == 666){
      process[table].pid = i;
      break;
    }
  }
  bool pipesfound = false;
  while(!pipesfound){
    process[table].pipein = pfind(process[table].pid, pid);
    uint32_t data = pread(process[table].pipein);
    if(data != 0){
      process[left].pid = data >> 4;
      process[right].pid = data & 15;
      for(int i = 0; i < 3; i++){
        mkfifo(pid, process[i].pid);
        popen(pid, process[i].pid);
        process[i].pipeout = pfind(pid, process[i].pid);
        pwrite(process[i].pipeout, 88);
        pipesfound = true;
      }
    }
  }
  bool allpipessetup = 0;
  while(allpipessetup < 2){
    allpipessetup = 0;
    for(int i = 0; i < 2; i++){
      process[i].pipein = pfind(process[i].pid, pid);
      if(pread(process[i].pipein) == 88){
        allpipessetup++;
      }
    }
  }
    write( STDOUT_FILENO, "Phill Readyyy: ", 15 );
  while(1);
  exit( EXIT_SUCCESS );
}
