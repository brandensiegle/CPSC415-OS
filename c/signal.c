/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <i386.h>

/* Your code goes here */
extern char* maxaddr;

typedef struct {
	unsigned long   edi;   
	unsigned long   esi;
	unsigned long   ebp;
	unsigned long   esp;
	unsigned long   ebx;
	unsigned long   edx;
	unsigned long   ecx;
	unsigned long   eax;
} newContext;

typedef struct {
	newContext ctx;
	unsigned long eip;
	unsigned long cs;
	unsigned long eflags;


	unsigned long returnAddress;
	void *handler;
	void *old_sp; 

	int currentSig;
} setup;

/*
 * Function:  sigtramp 
 * --------------------
 * Calls the appropriate handler. This function is never directly called
 *	but is added to the stack by signal().
 *
 *  @param:  Pointer to the handler function to run
 *  @param:  Pointer to the previous start of the process stack
 *  @return: none
 */
void sigtramp(void (*handler)(void *), void *cntx){
	handler(cntx);
	syssigreturn(cntx);
	return;
}

/*
 * Function:  signal 
 * --------------------
 * Verfies that a given signal can be sent to this process. If so,
 *	the stack is modified to run the trampoline code on next
 *	run
 *
 *  @param:  ID of the process to signal
 *  @param:  Signal to send
 *  @return: Status of operation
 */
int signal(int PID, int sig){
	pcb *process = getPcb(PID);
	
	if( ((process->sigMaskInstalled >> sig) & 0x1) == 0){
		return 0; //No handler installed, ignored
	} else {
		//check if higher signal running, if so add to lower priority queue
		//if no higher signal is running, setup stack so sigtramp code will execute
		if(((process->sigMaskOnStack >> sig) & 0xFFFFFFFF) > 1){
			if(((process->sigMaskQueued >> sig) & 0x0000001) == 1){
				//add this signal to sigMaskQueued
			} else {
				process->sigMaskQueued = process->sigMaskQueued + (0x1 << sig);
			}
			return 0;
		} else if (((process->sigMaskOnStack >> sig) & 0xFFFFFFFF) == 1){
			return 0;
		}
		
		//indicate that this process has been added to the stack
		process->sigMaskOnStack = process->sigMaskOnStack + (0x1 << sig);
		
		//set return values and cleanup dependant processes based
		// on the process' state
		switch (process->state){
		case READY:
		case RUNNING:
			ready(process);
			break;
		case WAITING:
			process->cpu_state.eax = -99;
			ready(process);
			break;
		case SLEEPING:
			removeFromSleepList(process);
			ready(process);
			break;
		case BLOCKED:
			cleanUpSenders(process);
			cleanUpRecvers(process);
			process->cpu_state.eax = -99;
			ready(process);
			break;
		}	



		//build the process stack to run the trampoline code upon next run
		unsigned long stkPtr = process->cpu_state.esp - sizeof(setup);
		for (int i=0;i<16;i++){ 
			if(i==0x8){ //Set the location of the process code
			*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = (volatile unsigned long) &sigtramp;
			} else if(i == 0x9){
				*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = getCS();
			} else if (i == 0xa) { 
				*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = 0x3200; 
			} else if (i == 0xb) {
				*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = (unsigned long) &sysstop;
			} else if (i == 0xc) { //When process runs off the end -> stop the process cleanly
				*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = (unsigned long) process->installedHandlers[sig];
			} else if (i == 0xd) { //When process runs off the end -> stop the process cleanly
				*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = (unsigned long) process->cpu_state.esp;
			} else if (i == 0xe) { //When process runs off the end -> stop the process cleanly
			*(volatile int*)(stkPtr+i*(sizeof(int))) = (int) sig;
			} else { 
				*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = i;
			} 
  		}
		//update the process stack pointer
		process->cpu_state.esp = process->cpu_state.esp - sizeof(setup);
	}

	return 0;
}

/*
 * Function:  handleRegister 
 * --------------------
 * Installs the provided signal handler and sends back a pointer to
 *	an old handler through void **oldHandler.
 *
 *  @param:  pointer to the process  pcb
 *  @param:  signal number
 *  @param:  pointer to handler function
 *  @param:  pointer to location to store old handler pointer
 *  @return: status of operation
 */
int handleRegister(pcb *p, int sig, void *newHandler, void **oldHandler){
	//check to make sure signal number is valid
	if(sig < 0 || sig > 30){
		return -1;
	}
	
	//Address check (hole, after memory) and return -2
	if(newHandler != NULL || ((unsigned long) newHandler > HOLESTART && (unsigned long) newHandler < HOLEEND) ||
		((unsigned long) oldHandler > HOLESTART && (unsigned long) oldHandler < HOLEEND) ||
		(unsigned long) newHandler > (unsigned long) maxaddr || (unsigned long) oldHandler > (unsigned long) maxaddr){
		return -2;
	}
	
	//if there was an old handler, send it back
	if ((int) p->installedHandlers[sig] != EMPTY){
		*oldHandler = p->installedHandlers[sig];
	}
	if(newHandler == NULL){
		if( ((p->sigMaskInstalled >> sig) & 0x1) == 1){
			p->sigMaskInstalled = p->sigMaskInstalled - (0x1 << sig);
		}

		p->installedHandlers[sig] = (void *) EMPTY;
	} else {
		//indicate the installation of a signal handler
		if( ((p->sigMaskInstalled >> sig) & 0x1) == 0){
			p->sigMaskInstalled = p->sigMaskInstalled + (0x1 << sig);
		}

		//set the handler in table
		p->installedHandlers[sig] = newHandler;
	}
	return 0;
}
