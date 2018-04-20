/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

/* Since we *know* there will be 2 processes, stemming from the 2 user
 * programs, we can
 *
 * - allocate a fixed-size process table (of PCBs), and then maintain
 *   an index into it for the currently executing process,
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to terminate.
 */

int pcbsize;  int amountofpcbs = 50; pcb_t pcb[50]; int executing; int changetime = 0; pipetype pipe[100];
int tablepid;

void scheduler(ctx_t* ctx) {
  bool doeschange = true;
  int oldexecuting = executing;
  int highestprioritypid;
  int highestpriority = -1;

  for(int i = 0; i < pcbsize; i++){
    int priorityi = pcb[i].priority;
    if(priorityi > highestpriority){
      highestpriority = priorityi;
      highestprioritypid = i;
      if(i != oldexecuting){
        doeschange = true;
      }
    }
    if(pcb[i].priority >= 0){
      pcb[i].priority += 1;
    }
  }
  executing = highestprioritypid;
  if(doeschange){
    memcpy(&(pcb[oldexecuting].ctx), ctx, sizeof(ctx_t)); // preserve P_1
    pcb[oldexecuting].status = STATUS_READY;                // update   P_1 status
    memcpy(ctx, &(pcb[executing].ctx), sizeof(ctx_t)); // restore  P_2
    pcb[executing].status = STATUS_EXECUTING;            // update   P_2 status
    pcb[executing].priority = 0;
  }
  return;
}

extern void     main_P3();
extern uint32_t tos;
extern void     main_P4();
extern void     main_console();
int pidcount;
//uint32_t ptos = (uint32_t)&tos - (50 * sizeof(pcb_t));


void hilevel_handler_rst(ctx_t* ctx) {
  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values match the entry point and top of stack.
   */

PL011_putc(UART0, 'R', true);
  memset( &pcb[0], 0, sizeof(pcb_t));
  pcb[0].pid      = 0;
  pcb[0].status   = STATUS_READY;
  pcb[0].ctx.cpsr = 0x50;
  pcb[0].ctx.pc   = (uint32_t)(&main_console);
  pcb[0].ctx.sp   = (uint32_t)(&tos);
  pcb[0].tos      = (uint32_t)(&tos);
  pcb[0].priority = 0;

  pcbsize = 1;
  pidcount = 1;

  for(int i = 0; i < 100; i++){
    pipe[i].inuse = false;
  }



  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

  memcpy(ctx, &pcb[0].ctx, sizeof(ctx_t));
  pcb[0].status = STATUS_EXECUTING;
  executing = 0;

  TIMER0->Timer1Load  = 0x00010000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();

  return;
}

void hilevel_handler_irq(ctx_t* ctx){

  uint32_t id = GICC0 -> IAR;

  if(id = GIC_SOURCE_TIMER0){
    scheduler(ctx); TIMER0->Timer1IntClr = 0x01;

  }

  GICC0->EOIR = id;
}


void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch(id) {
    case 0x00 : { // 0x00 => yield()
      scheduler(ctx);
      break;
    }
    case 0x01 : { // 0x01 => write( fd, x, n )
      int fd = (int)(ctx->gpr[0]);
      char* x = (char*)(ctx->gpr[1]);
      int n = (int)(ctx->gpr[2]);

      for(int i = 0; i < n; i++) {
        PL011_putc(UART0, *x++, true);
      }
      //ctx->gpr[0] = n;
      break;
    }
    case 0x03 : { // 0x03 => fork()
      uint32_t childid = -1;
      for(int i = 0; i < pcbsize; i++){
        if(pcb[i].status == STATUS_TERMINATED){
          childid = i;
          break;
        }
      }
      if(childid == -1){
        childid = pcbsize++;
      }
      memcpy( &pcb[executing].ctx, ctx, sizeof(ctx_t));
      memcpy( &pcb[childid], &pcb[executing], sizeof(pcb_t));
      pcb[childid].pid = pidcount++;
      pcb[childid].status = STATUS_READY;
      pcb[childid].tos = (uint32_t) &tos - (childid * 0x1000);
      uint32_t offsetstack = (uint32_t)(pcb[executing].tos - pcb[executing].ctx.sp);
      pcb[childid].ctx.sp = (uint32_t)(pcb[childid].tos - offsetstack);
      void *childstartofstack = (void *)(pcb[childid].tos - 0x00001000);
      void *parentstartofstack = (void *)(pcb[executing].tos - 0x00001000);
      memcpy(childstartofstack, parentstartofstack, 0x00001000);
      pcb[executing].ctx.gpr[0] = pcb[childid].pid;
      pcb[childid].ctx.gpr[0] = 0;
      memcpy(ctx, &pcb[executing].ctx, sizeof(ctx_t));
      break;
    }
    case 0x06 : { //0x04 => exit()
      uint32_t programid = (uint32_t)(ctx ->gpr[0]);
      for(int i = 0; i < pcbsize; i++){
        uint32_t findpid = pcb[i].pid;
        if(findpid == programid){
          void *startofstack = (void *)(pcb[i].tos - 0x00001000);
          memset(startofstack, 0, 0x00001000);
          memset(&pcb[i], 0, sizeof(pcb_t));
          pcb[i].status = STATUS_TERMINATED;
          if(i == pcbsize - 1){
            pcbsize--;
          }
          pcb[i].priority = -1;
        }
      }
      break;
    }
    case 0x05 : {  //0x05 => exec()
      uint32_t programcounter = (uint32_t)ctx -> gpr[0];
      //void *copyinto = (void *)pcb[executing].tos - 0x00001000;
      //memset(copyinto, 0, 0x00001000);
      ctx->pc = programcounter;
      ctx->sp = pcb[executing].tos;
      pcb[executing].priority = 0;
      break;
    }
    case 0x08 : {  //0x08 => mkfifo(startpid, endpid)
      uint32_t startpid = (uint32_t)ctx ->gpr[0];
      uint32_t endpid = (uint32_t)ctx ->gpr[1];
      for(int i = 0; i < 100; i++){
        if(pipe[i].inuse == false){
          //void *startofpipe = (void *)&pipe[i] - sizeof(pipetype);
          //memset(startofpipe, 0, sizeof(pipetype));
          pipe[i].inuse = true;
          pipe[i].write = startpid;
          pipe[i].read = endpid;
          break;
        }
      }
      break;
    }
    case 0x09 : { //0x09 => open(startpid, endpid)
      uint32_t startpid = (uint32_t)ctx ->gpr[0];
      uint32_t endpid = (uint32_t)ctx ->gpr[1];
      int pipeid = -1;
      for(int i = 0; i < 100; i++){
        if(pipe[i].write == startpid){
          if(pipe[i].read == endpid){
            pipeid = i;
            pipe[i].data = 0;
            break;
          }
        }
      }
      ctx ->gpr[0] = pipeid;
      break;
    }
    case 0x0A : { //0x0A => pwrite(pipenumber, data)
      int pipenumber = (uint32_t)ctx ->gpr[0];
      uint32_t data = (uint32_t)ctx ->gpr[1];
      if(pipe[pipenumber].write == executing){
        pipe[pipenumber].data = data;
      }
      break;
    }
    case 0x0B : { //0x0B => pfind(writepid, readpid)
      int writepid = (uint32_t)ctx ->gpr[0];
      int readpid = (uint32_t)ctx ->gpr[1];
      int pipenumber = -1;
      for(int i = 0; i < 100; i++){
        if(pipe[i].write == writepid){
          if(pipe[i].read == readpid){
            pipenumber = i;
            break;
          }
        }
      }
      ctx ->gpr[0] = pipenumber;
      break;
    }
    case 0x0C : { //0x0C => pread(pipenumber)
      int pipenumber = (uint32_t)ctx ->gpr[0];
      if(pipe[pipenumber].read == executing){
        uint32_t data = pipe[pipenumber].data;
        ctx ->gpr[0] = data;
      }
      break;
    }
    case 0x0D : { //0x0D => pclose(pipenumber)
      int pipenumber = (uint32_t)ctx ->gpr[0];
      pipe[pipenumber].data = 0;
      break;
    }
    case 0x0E : { //0x0E => punlink(pipenumber)
      int pipenumber = (uint32_t)ctx ->gpr[0];
      void *startofpipe = (void *)&pipe[pipenumber] - sizeof(pipetype);
      //memset(startofpipe, 0, sizeof(pipetype));
      pipe[pipenumber].inuse = false;
      pipe[pipenumber].data = 0;
      break;
    }
    case 0x0F : { //0x0F => getpid()
      ctx -> gpr[0] = pcb[executing].pid;
      break;
    }
    case 0x10 : { //0x10 => imtable()
      tablepid =  pcb[executing].pid;
    }
    case 0x11 : { //0x11 => findtable()
      ctx ->gpr[0] = tablepid;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }
  return;
}
