#include "waiter.h"

philosophers philosopher[16];
int forks[16]; //can only pick up fork n and n+1 e.g. if philosopher[4] can only pick up fork[4] and [5], -1 if noone picks up
int waiterpid = 1;
int amountoftimeseaten[16];
int kernelpipe;

#define leftforkdown 1
#define rightforkdown 2
#define pickforksup 3




void setup(){
  for(int i = 0; i < 16; i++){
    philosopher[i].pid = fork();
    exec(load("philosopher"));
    mkfifo(waiterpid, philosopher[i].pid);
    popen(waiterpid, philosopher[i].pid);
    philosopher[i].pipeout = pfind(waiterpid, philosopher[i].pid);
    pwrite(philosopher[i].pipeout, i);
    philosopher[i].status = thinking;
    philosopher[i].leftfork = 0;
    philosopher[i].rightfork = 0;
    forks[i] = -1;
    amountoftimeseaten[i] = 0;
  }
  mkfifo(waiterpid, 0);
  popen(waiterpid, 0);
  kernelpipe = pfind(waiterpid, 0);
  pwrite(kernelpipe, 666);
}
void main_waiter(){
  setup();
  yield();
  for(int i = 0; i < 16; i++){
    philosopher[i].pipein = pfind(philosopher[i].pid, waiterpid);
  }
  yield();
  while(1){
    for(int i = 0; i < 16; i++){
      uint32_t data = pread(philosopher[i].pipein);
      uint32_t leftfork = data >> 1;
      uint32_t rightfork = data & 1;
      if(forks[i] == i && leftfork == 0){
        forks[i] = -1;
        philosopher[i].leftfork = 0;
        puts("leftforkdown\n", 13);
      }
      else if(forks[i + 1] == i && rightfork == 0){
        forks[i+1] = -1;
        philosopher[i].rightfork = 0;
        puts("rightforkdown\n", 14);
      }
      else if(forks[i] == -1 && leftfork == 1){
        forks[i] = i;
        philosopher[i].leftfork = 1;
      }
      else if(forks[i+1] == -1 && rightfork == 1){
        forks[i+1] = i;
        philosopher[i].rightfork = 1;
      }
      else if(forks[i]== (i | -1) && forks[i+1] == (i+1|-1) && leftfork == 1 && rightfork == 1){
        forks[i] = i;
        forks[i+1] = i;
        philosopher[i].status = eating;
        philosopher[i].leftfork = 1;
        philosopher[i].rightfork = 1;
        amountoftimeseaten[i]++;
        puts("forksup\n",8);
        }
        uint32_t leftforkinuse = 0;
        if(forks[i] != (i | -1)){
          leftforkinuse = 1;
        }
        uint32_t rightforkinuse = 0;
        if(forks[i+1] != (i | -1)){
          rightforkinuse = 1;
        }
        uint32_t senddata = 1 << 4;
        senddata = senddata | (philosopher[i].leftfork << 3);
        senddata = senddata | leftforkinuse << 2;
        senddata = senddata | philosopher[i].rightfork << 1;
        senddata = senddata | rightforkinuse;
        pwrite(philosopher[i].pipeout, senddata);
      }
      yield();
    }
  }
