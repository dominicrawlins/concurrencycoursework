/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "libc.h"

int  atoi( char* x        ) {
  char* p = x; bool s = false; int r = 0;

  if     ( *p == '-' ) {
    s =  true; p++;
  }
  else if( *p == '+' ) {
    s = false; p++;
  }

  for( int i = 0; *p != '\x00'; i++, p++ ) {
    r = s ? ( r * 10 ) - ( *p - '0' ) :
            ( r * 10 ) + ( *p - '0' ) ;
  }

  return r;
}

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
     p++; t = -x; n = t;
  }
  else {
          t = +x; n = t;
  }

  do {
     p++;                    n /= 10;
  } while( n );

    *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}

void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  fork() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r)
              : "I" (SYS_FORK)
              : "r0" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

void exec( const void* x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
              :
              : "I" (SYS_EXEC), "r" (x)
              : "r0" );

  return;
}

int  kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}

void nice( int pid, int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  pid
                "mov r1, %2 \n" // assign r1 =    x
                "svc %0     \n" // make system call SYS_NICE
              :
              : "I" (SYS_NICE), "r" (pid), "r" (x)
              : "r0", "r1" );

  return;
}

void mkfifo( int pid, int endpid ) {
  asm volatile( "mov r0, %1 \n" // assign r0 = pid
                "mov r1, %2 \n" // assign r1 = x
                "svc %0     \n" // make system call SYS_MKFIFO
              :
              : "I" (SYS_MKFIFO), "r" (pid), "r" (endpid)
              : "r0", "r1" );

  return;
}

int  popen( int pid, int endpid ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 = endpid
                "svc %1     \n" // make system call SYS_OPEN
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_POPEN), "r" (pid), "r" (endpid)
              : "r0", "r1" );

  return r;
}

void pwrite( int pipenumber, uint32_t data ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  pipenumber
                "mov r1, %2 \n" // assign r1 =    data
                "svc %0     \n" // make system call SYS_PWRITE
              :
              : "I" (SYS_PWRITE), "r" (pipenumber), "r" (data)
              : "r0", "r1" );

  return;
}

int  pfind( int writepid, int readpid ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  writepid
                "mov r1, %3 \n" // assign r1 = readpid
                "svc %1     \n" // make system call SYS_PFIND
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_PFIND), "r" (writepid), "r" (readpid)
              : "r0", "r1" );

  return r;
}

uint32_t  pread( int pipenumber ) {
  uint32_t r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pipenumber
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r)
              : "I" (SYS_PREAD), "r" (pipenumber)
              : "r0");

  return r;
}

void pclose( int pipenumber ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  pipenumber
                "svc %0     \n" // make system call SYS_PCLOSE
              :
              : "I" (SYS_PCLOSE), "r" (pipenumber)
              : "r0" );

  return;
}
void punlink( int pipenumber ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  pipenumber
                "svc %0     \n" // make system call SYS_PUNLINK
              :
              : "I" (SYS_PUNLINK), "r" (pipenumber)
              : "r0" );

  return;
}
