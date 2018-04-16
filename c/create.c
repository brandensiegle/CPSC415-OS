/* create.c : create a process
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/*
 * Function:  create 
 * --------------------
 * Creates a process and adds it to the Process Control Block table if there is space and
 *	if there is enough memory for stack allocation. If the process creation fails for
 *	any reason then FAILED will be returned, otherwise OK is returned.
 *
 *  @param: Pointer to the process code
 *  @param: Size of memory to be allocated for process
 *  @return: ID of the process created
 */
extern int create(void (*func)(void), int memSize) {
  //find a free PCB slot for the new process
  pcb* newProcess = findFreePcb();
  // if no free pcb, return a FAILED
  if (newProcess == (void *) NULL) {
    return FAILED;
  }
  //allocate some memory for the process
  void* addr = kmalloc(memSize);
  if ((int) addr == EMPTY){
	//If there is no memory allocated, return a FAILED after giving back PCB
	removeProcess(newProcess);
	return FAILED;
  }

  //address of the base of stack
  unsigned long basePtr = (unsigned long) (addr+memSize)-16*sizeof(unsigned int);

  //move the stack pointer up 11 slots for registers and iret values storage
  unsigned long stkPtr = basePtr - 11*sizeof(unsigned long);

  //add allocated memory location to list of the process's resources
  memAlloc allocatedMemory;
  allocatedMemory.memLoc = addr;
  allocatedMemory.next = NULL;
  procResources resources;
  resources.memoryAllocs = allocatedMemory;

  //Add list of resources to PCB for new process
  newProcess->resources = resources;

  newProcess->runningTime = 0;

  //Initialize it to not be on any queue
  newProcess->next = NULL;
  newProcess->sleepNext = NULL;
  newProcess->sleepPrev = NULL;
  newProcess->sleepTime = 0;

  //Initialize it to not be waiting on anyone or have anyone waiting on it
  newProcess->waitingFor = 0;
  newProcess->waitingNext = NULL;
  
  //Initialize ipcArgs to null
  newProcess->ipcArgs = NULL;
  //Initialize target_of_recv to be own pid
  newProcess->target_of_recv = newProcess->pid;
  //Initialize sender_queue to be empty
  newProcess->sender_queue = NULL;
  // Initialize fdt
  initializeFDT(newProcess);


  //Initialize signal masks
  newProcess->sigMaskInstalled = 0x80000000;
  newProcess->sigMaskOnStack = 0x00000000;
  newProcess->sigMaskQueued = 0x00000000;

  //Initialize signal handlers
  memset(newProcess->installedHandlers,EMPTY,32*sizeof(void *));
  //memset(newProcess->previousHandlers,EMPTY,32*sizeof(void *));
  newProcess->installedHandlers[31] = &sysstop;

  //add to CPU state
  newProcess->cpu_state.ebp = basePtr;
  newProcess->cpu_state.esp = stkPtr;
  newProcess->cpu_state.iret_eip = (unsigned long) func;
  newProcess->cpu_state.iret_cs = getCS();

  //build the process' stack so it can return from an interrupt call
  for (int i=0;i<15;i++){ 
	if(i==0x8){ //Set the location of the process code
		*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = (volatile unsigned long) newProcess->cpu_state.iret_eip;
	} else if(i == 0x9){
		*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = getCS();
	} else if (i == 0xa) { 
		*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = 0x3200; 
	} else if (i == 0xb) { //When process runs off the end -> stop the process cleanly
		*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = (unsigned long) &sysstop;
	} else { 
		*(volatile unsigned long*)(stkPtr+i*(sizeof(unsigned long))) = 0x0;
	} 
  }

  //add the new process to the readyQueue
  ready(newProcess);

  //if successfully created, return OK
  return newProcess->pid;
}
