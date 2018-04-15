#include "table.h"

#define green 15

philosophers philosopher[16];
int forks[16];
int tpid;
int lowestpid;
int highestpid;
extern void main_philosopher();

int clamppid(int number){
  if(number == highestpid + 1){
    return lowestpid;
  }
  else if(number == lowestpid -1){
    return highestpid;
  }
  return number;
}

int clampnumber(int number){
  if(number == 16){
    return 0;
  }
  else if(number == -1){
    return 15;
  }
  else return number;
}

void main_table(){
  lowestpid = 51;
  highestpid = -1;
  write( STDOUT_FILENO, "Table Created: ", 15 );
  tpid = getpid();
  mkfifo(tpid, 0);
  popen(tpid, 0);
  int consolepipe = pfind(tpid, 0);
  pwrite(consolepipe, 666);
  for(int i = 0; i < 16; i++){
    pid_t cpid = fork();
    if(cpid == 0){
      exec(&main_philosopher);
    }
  }
  int amountfound = 0;
  while(amountfound < 16){
    for(int j = 0; j < 50; j++){
      int pipenumber = pfind(j,tpid);
      if(pipenumber > 0){
        int pipedata = pread(pipenumber);
        if(j == pipedata){
          pwrite(pipenumber, 0);
          philosopher[amountfound].pipein = pipenumber;
          philosopher[amountfound].pid = j;
          if(j > highestpid){
            highestpid = j;
          }
          if(j < lowestpid){
            lowestpid = j;
          }
          amountfound++;
          break;
        }
      }
    }
  }
  for(int i = 0; i < 16; i++){
    mkfifo(tpid, philosopher[i].pid);
    popen(tpid, philosopher[i].pid);
    philosopher[i].pipeout = pfind(tpid, philosopher[i].pid);
    uint32_t data = 0;
    data = data | (philosopher[clampnumber(i - 1)].pid << 6);
    data = data | (philosopher[clampnumber(i + 1)].pid);
    pwrite(philosopher[i].pipeout, data);
  }
  int processesready = 0;
  while(processesready < 16){
    for(int i =0; i < 16; i++){
      int ready = pread(philosopher[i].pipein);
      if(ready == green){
        pwrite(philosopher[i].pipein, 0);
        processesready++;
      }
    }
  }
  for(int i = 0; i< 16; i++){
    pwrite(philosopher[i].pipeout, green);
  }
  while(1){

    for(int i = 0; i < 16; i++){

    }
  }
  exit( EXIT_SUCCESS );
}
