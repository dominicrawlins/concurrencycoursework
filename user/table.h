#ifndef __TABLE_H
#define __TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libc.h"

typedef struct {
  int pid;
  int pipein;
  int pipeout;
} philosophers;

typedef enum{
  eating,
  thinking,
  ready,
}pstatus;

#endif
