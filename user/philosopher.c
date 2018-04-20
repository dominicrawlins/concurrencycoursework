#include "philosopher.h"

#define left 0
#define right 1
#define table 2
#define red -1
#define green 15
#define forkowned 3
#define forkused 4
#define forkavailable 5


void philprint(pstatus status, int inputpid, int phase, int roundseaten){
  write(STDOUT_FILENO, "Philosopher ",12);
  char* printpid;
  itoa(printpid, inputpid);
  write(STDOUT_FILENO, printpid, 2);
  if(status == thinking){
    write(STDOUT_FILENO, " thinking\n", 10);
  }
  else if(status == eating){
    write(STDOUT_FILENO, " eating\n", 8);
  }
  else if(status == ready){
    write(STDOUT_FILENO, " ready\n",7);
  }
  if(phase == 10 || phase == 50 || phase == 100){
    write(STDOUT_FILENO, "Rounds eaten: ", 14);
    char *eaten;
    itoa(eaten, roundseaten);
    write(STDOUT_FILENO, eaten, 3);
    write(STDOUT_FILENO, "\n\n", 2);
  }
}


void main_philosopher(){
  int pid;
  processes process[3];
  int fork[2] = {forkavailable, forkavailable};
  pstatus status = thinking;
  int phase = 0;
  int roundseaten = 0;


  pid = getpid();

  write( STDOUT_FILENO, "Philosopher ", 12 );
  char* ppid;
  itoa(ppid, pid);
  write(STDOUT_FILENO, ppid, 2);
  write(STDOUT_FILENO, " created\n", 9 );


  process[table].pid = findtable();
  mkfifo(pid, process[table].pid);
  popen(pid, process[table].pid);
  process[table].pipeout = pfind(pid, process[table].pid);
  char * pppid;
  itoa(pppid, pid);

  write(STDOUT_FILENO, "pppid\n", 2);
  pwrite(process[table].pipeout, pid);
  bool pipesfound = false;
  while(!pipesfound){

    int tablepipein = pfind(process[table].pid, pid);
    if(tablepipein > 0){

      process[table].pipein = tablepipein;
      uint32_t data = pread(process[table].pipein);
      if(data != 0){

        process[left].pid = data >> 6;
        process[right].pid = data & 63;
        for(int i = 0; i < 2; i++){
          mkfifo(pid, process[i].pid);
          popen(pid, process[i].pid);
          process[i].pipeout = pfind(pid, process[i].pid);
          pwrite(process[i].pipeout, 88);

        }
        pipesfound = true;
      }
    }
  }

  yield();
  int allpipessetup = 0;
  while(allpipessetup < 2){
    allpipessetup = 0;
    for(int i = 0; i < 2; i++){
      process[i].pipein = pfind(process[i].pid, pid);
      if(pread(process[i].pipein) == 88){
        allpipessetup++;
      }
    }
  }
  philprint(ready, pid, 0, 0);
  yield();
  pwrite(process[table].pipeout, 0);
  pwrite(process[left].pipeout, forkavailable);
  pwrite(process[right].pipeout, forkavailable);
  yield();
  while(1){
    int forkl = pread(process[left].pipein);
    int forkr = pread(process[right].pipein);
    if(forkl == forkused){
      fork[left] = forkowned;
    }
    else if(forkl == forkowned){
      fork[left] = forkused;
    }
    else if(forkl == forkavailable){
      fork[left] = forkavailable;
    }
    if(forkr == forkused){
      fork[right] = forkowned;
    }
    else if(forkr == forkowned){
      fork[right] = forkused;
    }
    else if(forkr == forkavailable){
      fork[right] = forkavailable;
    }
    if(fork[left] == forkowned && fork[right] == forkowned && (phase + pid)%5 == 0){
      fork[left] = forkavailable;
      fork[right] = forkavailable;
      status = thinking;
    }
    else if(fork[left] == forkavailable && fork[right] == forkavailable && (phase + pid)%5 == 0){
      fork[left] = forkowned;
      fork[right] = forkowned;
      status = eating;
    }
    if(status == eating){
      roundseaten++;
    }
    pwrite(process[left].pipeout, fork[left]);
    pwrite(process[right].pipeout, fork[right]);
    pwrite(process[table].pipeout, status);
    philprint(status, pid, phase, roundseaten);
    phase++;
    yield();
  }
  exit( EXIT_SUCCESS );
}
