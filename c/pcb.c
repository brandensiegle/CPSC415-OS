/* pcb.c : pcb table
*/

#include <xeroskernel.h>
#include <xeroslib.h>
#include <i386.h>

static pcb pcbTable[PCBTABLE_SIZE];	/* The PCB table of processes */
static int numRunning;			/* The number of running processes */
static int lastPID;			/* The PID of the last created process */
int str;

/*
 * Function:  pcbTableInit 
 * --------------------
 * Sets the value of the pcbTable to be filled with EMPTYs
 *
 *  @param: none
 *  @return: none
 */
void pcbTableInit( void ) {
  numRunning = 0;	
  lastPID = 0;
  memset(pcbTable, EMPTY, PCBTABLE_SIZE*sizeof(pcb)); 
  return;
}

/*
 * Function:  removeProcess 
 * --------------------
 * Removes a process from the PCB table while freeing any resources
 *	associatd with that process. The slot in the PCB table will
 *	be free for a new process.
 *
 *  @param: A pointer to the process that will be removed
 *  @return: none
 */
void removeProcess(pcb* process){
	/* Check to see if process has memory allocated */
	if(process->resources.memoryAllocs.memLoc == NULL){//no memory to free
		//clear the slot in pcb table and fill with empty
	} else {
		memHeader *headerToFree = process->resources.memoryAllocs.memLoc;
		kfree((void *)headerToFree);
		headerToFree = (memHeader *) process->resources.memoryAllocs.next;
		while(headerToFree != NULL){
			kfree((void *)headerToFree);
			headerToFree = (memHeader *) process->resources.memoryAllocs.next;
		}
	}
  

	//clear the slot in pcb table and fill with EMPTY
	memset(process, EMPTY, sizeof(process));		
	process->pid = TOMBSTONE;
	numRunning--;
	return;
}

/*
 * Function:  getFreePCBIndex 
 * --------------------
 * Gives the next free PID to a pcbTable entry and retruns the index
 *	of the entry in the PCB table.
 *
 *  @param: none
 *  @return: Index to the next free space in the PCB table
 */
int getFreePCBIndex(void){
	int newPID = lastPID;
	if(newPID>(PCBTABLE_SIZE-1)){newPID=1;}//wrap back to first PID
	int firstPIDChecked = newPID;
	
	do {
		Bool scanning = TRUE;
		
		//use hash function to find postion in PCB table based on PID
		int index = (newPID)%(PCBTABLE_SIZE-1);
		//always check if PID is already in use, increment newPID and try again
		while (scanning != FALSE){
			if(pcbTable[index].pid == EMPTY){
				lastPID = newPID+1;
				pcbTable[index].pid = newPID;
				return index;
			} else if(pcbTable[index].pid == TOMBSTONE){
				//Scan tombstones
				int possible = index;
				index++;
				while(newPID != pcbTable[index].pid){
					if(pcbTable[index].pid == EMPTY){
						lastPID = newPID+1;
						pcbTable[possible].pid = newPID;
						return possible;
					}
					if(possible == index){
						lastPID = newPID+1;
						return possible;
					}
					index++;
				}
				index = possible + 1;
				if(index>(PCBTABLE_SIZE-1)){index=1;}
			} else if(pcbTable[index].pid == newPID){
				scanning = FALSE;
			} else {
				index++;
				if(index>(PCBTABLE_SIZE-1)){index=1;}
			}
		}
		newPID++;
		if(newPID>(PCBTABLE_SIZE-1)){newPID=1;}//wrap back to first PID
	} while (newPID != firstPIDChecked);


	return -1;
}

/*
 * Function:  findFreePcb 
 * --------------------
 * Finds a place in the PCB table for a process. If there is no more room
 *	in the table then the function will return NULL
 *
 *  @param: none
 *  @return: a pointer to a free entry in the PCB table
 */
pcb* findFreePcb(void) {
	//If max number of processes is running, return NULL
	if (numRunning == PCBTABLE_SIZE){
		return NULL;
	}

	//get an index to the place in the PCB table
	int index = getFreePCBIndex();
	
	numRunning ++;
	
	return &pcbTable[index];
}



/*
 * Function:  getPcb 
 * --------------------
 * Iterates through the PCB table and returns a pointer to the PCB with
 *	the requested PID.
 *
 *  @param: PID of the PCB
 *  @return: Pointer to PCB entry or SYSERR if not found.
 */
pcb *getPcb( PID_t id) {
	int stopCounter = 0;

	//use hash function to determine where pcb should be
	int index = (id)%(PCBTABLE_SIZE-1);
	
	if(pcbTable[index].pid == id){return &pcbTable[index];}
	else{stopCounter++;index++;}
	
	while (stopCounter < PCBTABLE_SIZE){
		if(pcbTable[index].pid == EMPTY){return (void *) SYSERR;}
		else if(pcbTable[index].pid == id){return &pcbTable[index];}
		else{stopCounter++;index++;}
	}

  	return (void *) SYSERR;
};

/*
 * Function:  printPcbTable 
 * --------------------
 * Iterates through the PCB table and prints the information for each entry.
 *
 *  @param: none
 *  @return: none
 */
void printPcbTable( void ) {
  kprintf("#PCB.c -->\n");
  pcb *pcbEntry;
  //loop through each entry and calprintPcb() for each
  for (int i =0; i < 32; i++){
    pcbEntry = &pcbTable[i];
    kprintf(" #[%d] ", i);	
    if(pcbEntry != (pcb*) SYSERR){
    printPcb(pcbEntry);
    } else {
    	kprintf("\n");
    }
	for(int i=0;i<100000;i++){}
  }

  kprintf("\n"); 
};

/*
 * Function:  printPcb 
 * --------------------
 * Prints the pid, %esp, and %eip for a given entry in a PCB table.
 *
 *  @param: A pointer to an entry in a PCB table
 *  @return: none
 */
void printPcb(pcb* pcbEntry) {
	kprintf("id:0x%x, esp:0x%x, eip:0x%x, ", pcbEntry->pid, pcbEntry->cpu_state.esp, pcbEntry->cpu_state.iret_eip);
	//sysputs(str);
	//for(;;){}
    	kprintf("next-> addr: %d\n",pcbEntry->next);
}

/*
 * Function:  getAllRecvers
 * --------------------
 *  Helper function for c/msgQueues.c:cleanUpRecvers
 *  Iterate through PCBTable to find processes receiving on cleanUpPid
 *
 *  @param:  Pid of process to be cleaned up
 *  @param:  Pid_t array of size PCBTABLE_SIZE to store receiver list
 *           the array will be terminated by a NULL
 *  @return: OK
 */
int getAllRecvers(PID_t cleanUpPid, PID_t *pidArray) {
  int i = 0;
  pcb *entry;
  for (int j=0;j<PCBTABLE_SIZE;j++) {
    entry = &pcbTable[j];

    if ((((PID_t) entry->target_of_recv) == cleanUpPid) && \
            ((PID_t) entry->pid != cleanUpPid)) {
      pidArray[i] = entry->pid;
      i++;  
    }
  }

  pidArray[i] = NULL; //termination symbol
  return OK;
}
extern char * maxaddr;
/*
 * Function:  getCPUtimes
 * --------------------
 *  Fill processStatuses struct with pcb table information
 *
 *  @param:  Pcb of current active process
 *  @param:  Pointer to a processStatuses struct to be modified.
 *  @return: Value of last slot used
 *           -1 if the address is in the hole
 *           -2 if the address goes beyond main memory
 */
int getCPUtimes(pcb *p, processStatuses *ps) {
  int i, currentSlot;
  currentSlot = -1;
  // Check if address is in the hole
  if (((unsigned long) ps) >= HOLESTART && ((unsigned long) ps <= HOLEEND)) 
    return -1;

  //Check if address of the data structure is beyone the end of main memory
  if ((((char * ) ps) + sizeof(processStatuses)) > maxaddr)  
    return -2;
  
  for (i=0; i<PCBTABLE_SIZE; i++) {
    if (pcbTable[i].state != STOPPED && pcbTable[i].pid != TOMBSTONE) { 
      currentSlot++;
      ps->pid[currentSlot] = pcbTable[i].pid;
      ps->status[currentSlot] = \
        p->pid == pcbTable[i].pid ? RUNNING : pcbTable[i].state;
      ps->cpuTime[currentSlot] = pcbTable[i].runningTime;

    }
  }

  return currentSlot;
}

void addCPUTimes(int milliseconds){
  int i, currentSlot;
  currentSlot = -1;
	for (i=0; i<PCBTABLE_SIZE; i++) {
    		if (pcbTable[i].state != STOPPED && pcbTable[i].pid != TOMBSTONE) { 
      			currentSlot++;
			pcbTable[i].runningTime += milliseconds;
		}
  	}
}

int initializeFDT(pcb *process) {
  int i;
  for (i=0; i<FDT_SIZE; i++) {
    //process->FD_tab[i].dvBlock = NULL;
    clearFD(process, i);
  }
  return OK;
}

int findFreeFD(pcb *process) {
  // return index of the first available file descriptor
  // if none is available, EMPTY is returned
  int i;
  for (i=0; i<FDT_SIZE; i++) {
    if (process->FD_tab[i].dvBlock == NULL) {
      return i;
    }
  }
  return EMPTY;
}

int fillFD(pcb *process, int fd, devsw *devptr) {
  // fill fd of process with device pointer
  // If successful, OK is returned
  // If fd is beyond fdt size, FAILED is returned
  // If fd is in use, FAILED is returned
  if (fd < FDT_SIZE) {
    if (process->FD_tab[fd].dvBlock == (devsw *) NULL) {
      process->FD_tab[fd].dvBlock = devptr;
      return OK;
    } else {
      return FAILED;
    }
  } else {
    return FAILED;
  }
}

void clearFD(pcb *process, int fd) {
  if (fd < FDT_SIZE) {
    process->FD_tab[fd].dvBlock = (devsw *) NULL;
  }
}
