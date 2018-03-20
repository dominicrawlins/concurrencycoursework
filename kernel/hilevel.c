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

int pcbsize; pcb_t pcb[ 50 ]; int executing = 0; int changetime = 0;

void scheduler( ctx_t* ctx ) {
  /*for(int i = 0; i < pcbsize; i++){
    if(pcb[i].priority < pcb[executing].priority){
      memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) ); // preserve P_1
      pcb[ executing ].status = STATUS_READY;                // update   P_1 status
      memcpy( ctx, &pcb[ i ].ctx, sizeof( ctx_t ) ); // restore  P_2
      pcb[ i ].status = STATUS_EXECUTING;            // update   P_2 status
      executing = i;                                 // update   index => P_2
    }
  }
  changetime++;
  if(changetime % 5 == 0){
    pcb[executing].priority = pcb[executing].priority + 1;
  }

*/
 int oldexecuting = -1;
 if(executing == pcbsize - 1){
   oldexecuting = executing;
   executing = 0;
 }
 else{
   oldexecuting = executing;
   executing++;
 }
 memcpy( &(pcb[ oldexecuting ].ctx), ctx, sizeof( ctx_t ) ); // preserve P_1
 pcb[ oldexecuting ].status = STATUS_READY;                // update   P_1 status
 memcpy( ctx, &(pcb[ executing ].ctx), sizeof( ctx_t ) ); // restore  P_2
 pcb[ executing ].status = STATUS_EXECUTING;            // update   P_2 status


  return;
}

extern void     main_P3();
extern uint32_t tos;
extern void     main_P4();
extern void     main_console();
int pidcount;


void hilevel_handler_rst( ctx_t* ctx              ) {
  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values match the entry point and top of stack.
   */

PL011_putc(UART0, 'R', true);
  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 0;
  pcb[ 0 ].status   = STATUS_READY;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos);
  pcb[ 0 ].tos      = ( uint32_t )( &tos);
  pcb[ 0 ].priority = 0;

  /*memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 1 ].pid      = 2;
  pcb[ 1 ].status   = STATUS_READY;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( &main_P3 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )( &tos + (pcb[ 1 ].pid * 0x00001000   ));
  pcb[ 1 ].priority = 3;

  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 2 ].pid      = 3;
  pcb[ 2 ].status   = STATUS_READY;
  pcb[ 2 ].ctx.cpsr = 0x50;
  pcb[ 2 ].ctx.pc   = ( uint32_t )( &main_P4 );
  pcb[ 2 ].ctx.sp   = ( uint32_t )( &tos + (pcb[ 2 ].pid * 0x00001000 ));
  pcb[ 2 ].priority = 3;
*/
  pcbsize = 1;
  pidcount = 1;


  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

  memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
  pcb[ 0 ].status = STATUS_EXECUTING;
  executing = 0;

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
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

void hilevel_handler_irq( ctx_t* ctx){

  uint32_t id = GICC0 -> IAR;

  if(id = GIC_SOURCE_TIMER0){
    scheduler(ctx); TIMER0->Timer1IntClr = 0x01;

  }

  GICC0->EOIR = id;
}


void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      scheduler( ctx );
      break;
    }

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );
      char*  x = ( char* )( ctx->gpr[ 1 ] );
      int    n = ( int   )( ctx->gpr[ 2 ] );

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }

      //ctx->gpr[ 0 ] = n;
      break;
    }


    case 0x03 : { // 0x03 => fork()
      uint32_t childid = -1;
      for(int i = 0; i < pcbsize; i++){
        if(pcb[i].status == STATUS_TERMINATED){
          childid = i;
        }
      }
      if(childid == -1){
        childid = pcbsize++;
      }
      memcpy( &pcb[executing].ctx, ctx, sizeof(ctx_t));
      memcpy( &pcb[childid], &pcb[executing], sizeof( pcb_t ) );
      pcb[childid].pid      =  pidcount++;
      pcb[childid].status   = STATUS_CREATED;
      pcb[childid].tos      = (uint32_t) &tos - (childid * 0x1000);
      uint32_t offsetstack = (uint32_t)(pcb[executing].tos - pcb[executing].ctx.sp);
      pcb[childid].ctx.sp = (uint32_t)(pcb[childid].tos - offsetstack);
      void *childstartofstack = (void *)(pcb[childid].tos - 0x00001000);
      void *parentstartofstack = (void *)(pcb[executing].tos - 0x00001000);
      memcpy(childstartofstack, parentstartofstack, 0x00001000);
      pcb[executing].ctx.gpr[ 0 ] = pcb[childid].pid;
      pcb[childid].ctx.gpr[ 0 ] = 0;
      memcpy(ctx, &pcb[executing].ctx, sizeof(ctx_t));
      break;
    }
    case 0x06 : { //0x04 => exit()
      uint32_t programid = (uint32_t)(ctx ->gpr[0]);
      uint32_t processnumbertodelete = -1;
      for(int i = 0; i < pcbsize; i++){
        uint32_t findpid = pcb[i].pid;
        if(findpid == programid){
          processnumbertodelete = i;
        }
      }
      if(processnumbertodelete > -1){
        void *startofstack = (void *)(pcb[processnumbertodelete].tos - 0x00001000);
        memset(startofstack, 0, 0x00001000);
        memset(&pcb[processnumbertodelete], 0, sizeof(pcb_t));
        pcb[processnumbertodelete].status =   STATUS_TERMINATED;
        if(processnumbertodelete == pcbsize - 1){
          pcbsize--;
        }
      }
      break;
    }
    case 0x05 : {  //0x05 => exec()
      uint32_t programcounter = (uint32_t)ctx -> gpr[0];
      void *copyinto = (void *)pcb[executing].tos - 0x00001000;
      memset(copyinto, 0, 0x00001000);
      ctx->pc = programcounter;
      ctx->sp = pcb[executing].tos;
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
