#include "philosopher.h"
int pid;
int pipein;
int pipeout;
int leftfork;
int rightfork;
int round;
bool atelasttime;


void main_philosopher(){
  for(int i = 0; i < 100; i++){
    pipein = pfind(1, i);
    if(pipein > -1){
      if(pread(pipein) != 0){
        pid = i;
        break;
      }
    }
  }
  pwrite(pipein, 0);
  mkfifo(pid, 1);
  popen(pid, 1);
  pipeout = pfind(pid, 1);
  int round = 0;
  leftfork = 0;
  rightfork = 0;
  atelasttime = false;
  round = 0;
  yield();
  while(1){
    uint32_t data = pread(pipein);
    pwrite(pipein, 0);
    uint32_t datareceived = data >> 4;
    leftfork = data >> 3 & 1;
    uint32_t leftforktaken = data >> 2 & 1;
    rightfork = data >> 1 & 1;
    uint32_t rightforktaken = data & 1;
    if(datareceived == 0){
      yield();
    }
    if(leftfork == 0 && rightfork == 1){
      if(atelasttime){
        rightfork = 0;
      }
      else if(leftforktaken == 0){
        leftfork = 1;
      }
      else{
        rightfork = 0;
      }
      atelasttime = false;
    }
    else if(leftfork == 1 && rightfork == 0){
      if(atelasttime){
        leftfork = 0;
      }
      else if(rightforktaken = 0){
        rightfork = 1;
      }
      else{
        leftfork = 0;
      }
      atelasttime = false;
    }
    else if(leftfork == 0 && rightfork == 0){
      if(leftforktaken == 0 && rightforktaken == 0){
        leftfork = 1;
        rightfork = 1;
      }
      atelasttime = false;
    }
    else if(leftfork == 1 && rightfork == 1){
      if((round + pid) % 5 == 0){
      leftfork = 0;
      atelasttime = true;
      }
    }
    round++;
    uint32_t databack = 1 << 2;
    databack = databack | leftfork << 1;
    databack = databack | rightfork;
    pwrite(pipeout, databack);
  }
}
