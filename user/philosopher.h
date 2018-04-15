#ifndef __PHILOSOPHER_H
#define __PHILOSOPHER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"


typedef struct {
  int pid;
  int pipein;
  int pipeout;
} processes;

typedef enum{
  eating,
  thinking,
}pstatus;
#endif
