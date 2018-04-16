/* ctsw.c : context switcher
 */
#include <i386.h>
#include <xeroskernel.h>

/* Your code goes here - You will need to write some assembly code. You must
   use the gnu conventions for specifying the instructions. (i.e this is the
   format used in class and on the slides.) You are not allowed to change the
   compiler/assembler options or issue directives to permit usage of Intel's
   assembly language conventions.
*/
void _ISREntryPoint(void);		/* Entry point into the InteruptServiceRoutine */
void _TimerEntryPoint(void);	/* Entry point into the InterruptEntryPoint */
void _KBDEntryPoint(void);    /* Entry point for keyboard interrupts */
void _CommonJump(void);			/* Entry point into the rest of context switch code */
static void *k_stack;			/* Variable to keep track of kernel stack */
static unsigned long *ESP;		/* Pointer to the process' stack */
static unsigned long rc;	/* The call number to provide to dispatcher */
static unsigned long retValue;
static unsigned long isInterrupt;
static long args;



/*
 * Function:  contextinit 
 * --------------------
 * A1) Sets up the Interupt Service Rountine to have entry points for CREATE, YIELD, and
 *	STOP system calls.
 * A2) Sets up the SYSKILL,SYSPUTS, and GETPID system call.
 *
 *  @param: none
 *  @return: none
 */
void contextinit( void ) {
  set_evec(CREATE, (unsigned long) _ISREntryPoint);
  set_evec(YIELD, (unsigned long) _ISREntryPoint);
  set_evec(STOP, (unsigned long) _ISREntryPoint);
  set_evec(KILL, (unsigned long) _ISREntryPoint);
  set_evec(PUTS, (unsigned long) _ISREntryPoint);
  set_evec(GETPID, (unsigned long) _ISREntryPoint);
  set_evec(TIMER_INT, (unsigned long) _TimerEntryPoint);
  set_evec(KBD_INT, (unsigned long) _KBDEntryPoint);
  initPIT(100);
}


/*
 * Function:  contextswitch 
 * --------------------
 * Handles switching from kernel space to the process space. The process will run
 *	until interrupted and returns here via the ISR.
 *
 *  @param: The current process to be switched to
 *  @return: The call number of the interrupt
 */
int contextswitch(pcb* process){
  ESP = (unsigned long *) process->cpu_state.esp;
  retValue = process->cpu_state.eax;
  __asm __volatile( " \
    pushf   \n\
    pusha   \n\
    movl %%esp, k_stack   \n\
    movl ESP, %%esp   \n\
    popa    \n\
    movl retValue, %%eax	\n\
    iret    \n\
  _TimerEntryPoint: \n\
    cli  \n\
    pusha  \n\
    movl   $1, %%ecx  \n\
    movl   $32, %%eax  \n\
    jmp     _CommonJump  \n\
  _KBDEntryPoint: \n\
    cli \n\
    pusha \n\
    movl  $1, %%ecx  \n\
    movl  $33, %%eax  \n\
    jmp     _CommonJump  \n\
  _ISREntryPoint:  \n\
    cli  \n\
    pusha  \n\
    movl    $0, %%ecx  \n\
    movl %%edx, args \n\
  _CommonJump:   \n\
    movl %%esp, ESP   \n\
    movl k_stack, %%esp   \n\
    movl  %%eax, rc  \n\
    movl  %%ecx, isInterrupt  \n\
    popa    \n\
    popf    \n\
            "
    :
    :
    : "%eax", "%ecx"
    );
  if (isInterrupt==1) {
	
  } else { 
	process->arguments = args;
  }

    process->cpu_state.esp = (unsigned long) ESP;
    if(k_stack){} // prevent compiler warnings
  return rc;
}
