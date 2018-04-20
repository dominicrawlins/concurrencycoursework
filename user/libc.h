/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __LIBC_H
#define __LIBC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Define a type that that captures a Process IDentifier (PID).

typedef int pid_t;

/* The definitions below capture symbolic constants within these classes:
 *
 * 1. system call identifiers (i.e., the constant used by a system call
 *    to specify which action the kernel should take),
 * 2. signal identifiers (as used by the kill system call),
 * 3. status codes for exit,
 * 4. standard file descriptors (e.g., for read and write system calls),
 * 5. platform-specific constants, which may need calibration (wrt. the
 *    underlying hardware QEMU is executed on).
 *
 * They don't *precisely* match the standard C library, but are intended
 * to act as a limited model of similar concepts.
 */

#define SYS_YIELD     ( 0x00 )
#define SYS_WRITE     ( 0x01 )
#define SYS_READ      ( 0x02 )
#define SYS_FORK      ( 0x03 )
#define SYS_EXIT      ( 0x04 )
#define SYS_EXEC      ( 0x05 )
#define SYS_KILL      ( 0x06 )
#define SYS_NICE      ( 0x07 )
#define SYS_MKFIFO    ( 0x08 )
#define SYS_POPEN     ( 0x09 )
#define SYS_PWRITE    ( 0x0A )
#define SYS_PFIND     ( 0x0B )
#define SYS_PREAD     ( 0x0C )
#define SYS_PCLOSE    ( 0x0D )
#define SYS_PUNLINK   ( 0x0E )
#define SYS_GETPID    ( 0x0F )
#define SYS_IMTABLE   ( 0x10 )
#define SYS_FINDTABLE ( 0x11 )

#define SIG_TERM      ( 0x00 )
#define SIG_QUIT      ( 0x01 )

#define EXIT_SUCCESS  ( 0 )
#define EXIT_FAILURE  ( 1 )

#define  STDIN_FILENO ( 0 )
#define STDOUT_FILENO ( 1 )
#define STDERR_FILENO ( 2 )

// convert ASCII string x into integer r
extern int  atoi( char* x        );
// convert integer x into ASCII string r
extern void itoa( char* r, int x );

// cooperatively yield control of processor, i.e., invoke the scheduler
extern void yield();

// write n bytes from x to   the file descriptor fd; return bytes written
extern int write( int fd, const void* x, size_t n );
// read  n bytes into x from the file descriptor fd; return bytes read
extern int  read( int fd,       void* x, size_t n );

// perform fork, returning 0 iff. child or > 0 iff. parent process
extern int  fork();
// perform exit, i.e., terminate process with status x
extern void exit(       int   x );
// perform exec, i.e., start executing program at address x
extern void exec( const void* x );

// for process identified by pid, send signal of x
extern int  kill( pid_t pid, int x );
// for process identified by pid, set  priority to x
extern void nice( pid_t pid, int x );

//create space for a pipe, set start and end pid
extern void mkfifo( pid_t pid, pid_t endpid );

//open pipe, return pipe id
extern int popen( pid_t pid, pid_t endpid );

//write data through pipe
extern void pwrite( int pipenumber, uint32_t data);

//find pipe to read from
extern int pfind( int writepid, int readpid);

//read from pipe
extern uint32_t pread( int pipenumber);

//close pipe
extern void pclose(int pipenumber);

//delete pipe
extern void punlink(int pipenumber);

//getpid
extern int getpid();

//declares this pid is the table pid
extern void imtable();

//finds tables pid
extern int findtable();


#endif
