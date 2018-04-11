#ifndef __WAITER_H
#define __WAITER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "PL011.h"

#include "libc.h"


typedef enum {
  eating,
  thinking
} pstatus;


typedef struct{
  pstatus status;
  int leftfork; //0 for down, 1 for in use
  int rightfork;
  int pid;
  int pipein;
  int pipeout;
} philosophers;


#endif
