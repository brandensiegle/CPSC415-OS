/* disp.c : dispatcher
 */
#include <i386.h>
#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */
static pcb *readyQueue;		/* A pointer to the first item on the ready queue */
va_list* vArgsPtr;		/* Pointer to a list of arguments */
va_list vArgs;			/* List of arguments */
static pcb* process;			/* the current running process */

/*
 * Function:  initializeQueues 
 * --------------------
 * Sets the readyQueue pointer to be initially NULL
 *
 *
 *  @param: none
 *  @return: none
 */
void initializeQueues(void) {
	readyQueue = NULL;
}


/*
 * Function:  dispatch 
 * --------------------
 * Continually loops as processes request services and interrupts occur. This handles
 *	the calls for creation of new processes, yielding of processes, and termination
 *	of processes. 
 *
 *  @param: none
 *  @return: none
 */
void dispatch(void){
    int request;	/* the service value requested */
    void* buffer; /* variables for send, recv */
    int buffer_len; /* variables for send, recv */
    int fd; /* variables for device calls */ 

    process = next();

    for( ;; ){
        request = contextswitch(process);
	
        switch( request ){
        //create a process
        case( CREATE ):{
			//get args from pcb
			vArgs = (va_list) process->arguments;

			//Pull address of process code off stack
			void* codeAddr = va_arg(vArgs,void*);
			//Pull size of process allocation off stack
			int pSize = va_arg(vArgs,int);

			//create the process requested
			process->cpu_state.eax = create(codeAddr, pSize);
        break;}
            
        //yield the current process
        case( YIELD ):	
	    ready( process ); 
            process = next(); 
        break;
            
        //terminate the current process
        case( STOP ): 
			cleanup( process );
			process = next();
        break;

            //terminate the selected process
        case( KILL ):{ 
			vArgs = (void *) process->arguments;

			//Pull PID and signal number of process off stack
			int procID = va_arg(vArgs,int);
			int signalNum = va_arg(vArgs,int);

			//Get the process to signal
			pcb* procToSignal = getPcb(procID);

			if(procToSignal == (void *) SYSERR || procID <= 0 || procID >= PCBTABLE_SIZE){//check if process exists
				process->cpu_state.eax = -512;
			} else if (signalNum < 0 || signalNum > 31){//check to see if valid signal number
				process->cpu_state.eax = -561;
			} else {//else, handle signal
				process->cpu_state.eax = signal(procID,signalNum);
			}
			
					
        break;}

        //Output to the screen
        case( PUTS ): 
			for(int i = 0; i< 1;i++){}
			vArgsPtr =(va_list*) (process->cpu_state.esp + 5*sizeof(unsigned long));
			vArgs = *vArgsPtr;
		
			//Pull string off stack
			char *str = va_arg(vArgs,char*);
		
			//Print the string
			kprintf(str);

        break;

        //Returns the pid of the running process
        case( GETPID ): 
			process->cpu_state.eax = process->pid;
        break;

	case( TIMER_INT ):
	    tick();
	    addCPUTimes(TIME_SLICE);
	    ready( process ); 
            process = next(); 
	    end_of_intr();
	break;
         

    case( SEND ):
		vArgsPtr = (va_list *) &process->arguments;
		// preserve va_list state for send
		va_copy(vArgs, *vArgsPtr); 
		process->ipcArgs = vArgsPtr;
		PID_t dest_pid = va_arg(vArgs, PID_t);
		buffer = va_arg(vArgs, void*);
		buffer_len = va_arg(vArgs, int);
		process->cpu_state.eax = send(dest_pid, buffer, buffer_len, process);
		va_end(vArgs); // dealloc copy of va_list
		process = next();
    break;

	case( RECV ):
		    vArgsPtr = (void *) &process->arguments;
		    // preserve va_list state for recv
		    va_copy(vArgs, *vArgsPtr);
		    process->ipcArgs = vArgsPtr;
		    PID_t *from_pid = va_arg(vArgs, PID_t*);
		    buffer = va_arg(vArgs, void*);
		    buffer_len = va_arg(vArgs, int);
		    process->cpu_state.eax = recv(from_pid, buffer, buffer_len, process);
		    va_end(vArgs); // dealloc copy of va_list
		    process = next();
	break;

	case ( SLEEP ):
		vArgs = (void *) process->arguments;
		
		//Pull time off stack
		unsigned int milliseconds = va_arg(vArgs,unsigned int);

		sleep(milliseconds);
		process = next();
	break;

	case (SIGHANDLE):{
		//get arguments from system call
		vArgsPtr = (void *) &process->arguments;
		va_copy(vArgs, *vArgsPtr);
		
		
		int singalNum = va_arg(vArgs, int);
		void *newHandler = va_arg(vArgs, int);
		void **oldHandler = va_arg(vArgs, int);
		

		//call function in signal.c
		process->cpu_state.eax = handleRegister(process, singalNum, newHandler, oldHandler);
		
		//don't get next process, just go back into the same process
	break;}

	case (WAIT): //Blocks the current process and has it wait for the specified process to end
		//get arguments from system call
		vArgsPtr = (void *) &process->arguments;
		va_copy(vArgs, *vArgsPtr);

		int waitPID = va_arg(vArgs, int);

		//basic deadlock and non-existant process checks
		if(waitPID < 1 || waitPID == process->pid || waitPID >= PCBTABLE_SIZE){
			process->cpu_state.eax = -1;
			break;
		}

		//Sets process.waitingFor to be the specified value
		process->waitingFor = waitPID;


		//adds current process to end of waitee's waiting list
		pcb *listStart = getPcb(waitPID);
	
		//if process doesnt exist
		if(listStart == (void *) SYSERR){
			process->cpu_state.eax = -1;
			break;
		}

		//kprintf(listStart
		while (listStart->waitingNext != NULL){
			listStart = listStart->waitingNext;
			//kprintf("Loop\n");
		}
		listStart->waitingNext = process;
		

		//Sets process.state to be WAITING
		process->state = WAITING;
		process->cpu_state.eax = 0;
		//gets next process
		process = next();

	break;

	case (SIGRETURN): 
		//get arguments from system call
		vArgsPtr = (void *) &process->arguments;
		va_copy(vArgs, *vArgsPtr);

		void *old_sp = va_arg(vArgs, void *);

		//clear the last signal from the signal stack
		int lastSig = *(volatile int*)(old_sp-sizeof(int));
		process->sigMaskOnStack = process->sigMaskOnStack -  (0x1 << lastSig);

		process->cpu_state.esp = (unsigned long)old_sp;

		//check for queued signals
		for (int i=31;i>=0;i--){
			if(((process->sigMaskQueued >> i) & 0x0000001) == 1){ //find the first item on queue
				if(((process->sigMaskOnStack >> i) & 0xFFFFFFFF) > 0){ //is something higher on stack
					break;
				} else {
					process->sigMaskQueued = process->sigMaskQueued -  (0x1 << i);
					signal(process->pid,i);
					break;
				}
			}
		}

	break;
	
  case ( GETCPUTIMES ):
    vArgs = (va_list) process->arguments;
    process->cpu_state.eax = getCPUtimes(process, va_arg(vArgs, processStatuses *));

  break;

  case ( DV_OPEN ):
    vArgs = (va_list) process->arguments;
    int device_no = va_arg(vArgs, int);
    process->cpu_state.eax = di_open(process, device_no);
    process = next();
  break;
  
  case ( DV_CLOSE ):
    vArgs = (va_list) process->arguments;
    fd = va_arg(vArgs, int);
    process->cpu_state.eax = di_close(process, fd);
    process = next();
  break;
  
  case ( DV_WRITE ):
    vArgs = (va_list) process->arguments;
    fd = va_arg(vArgs, int);
    buffer = va_arg(vArgs, void*);
    buffer_len = va_arg(vArgs, int);
    process->cpu_state.eax = di_write(process, fd, buffer, buffer_len);
    process = next();
  break;

  case ( DV_READ ):
    vArgs = (va_list) process->arguments;
    fd = va_arg(vArgs, int);
    buffer = va_arg(vArgs, void*);
    buffer_len = va_arg(vArgs, int);
    process->cpu_state.eax = di_read(process, fd, buffer, buffer_len);
    process = next();
  break;

  case ( DV_IOCTL ):
    vArgs = (va_list) process->arguments;
    fd = va_arg(vArgs, int);
    unsigned long command = va_arg(vArgs, unsigned long);
    va_list *ioctl_args = va_arg(vArgs, va_list);
    process->cpu_state.eax = di_ioctl(process, fd, command, (void*) ioctl_args);
    process = next();
  break;

  case ( KBD_INT ):
    ;
    //unsigned char bb = inb(KBD_DP);
    //unsigned char cc = inb(KBD_SP);
    //kprintf("K:DISP: kbd_int, 0x%X\n",cc);
    //kprintf("K:DISP: kbd_int, 0x%X, 0x%X\n", bb, cc);
    handleKbdISR();
	  end_of_intr();
	  ready( process ); 
    process = next(); 
  break;
        }
    }
}



/*
 * Function:  next 
 * --------------------
 * Gets the next process available from the ready queue. If the ready queue
 *	is empty then the system will lock for now as there is no NULL
 *	process created.
 *
 *  @param: none
 *  @return: A pointer to the next process on the ready queue.
 */
pcb* next( void ) {
  pcb *nextProcess = NULL;
  
  //If the queue is empty, inform the user and loop indefinitly
  if(readyQueue == NULL){
	return getPcb(0);
  }

  nextProcess = readyQueue;

  if(readyQueue->next != NULL){ //if there is a next item, make that the new head of the readyQueue
  	readyQueue = readyQueue->next;
	readyQueue->prev = NULL;

  } else {//if there is nothing after, leave queue empty
	readyQueue = NULL;
  }
  
  nextProcess->next = NULL;
  nextProcess->prev = NULL;
  nextProcess->state = RUNNING;
  return nextProcess;
}

/*
 * Function:  ready 
 * --------------------
 * Adds the process to the end of the ready queue.
 *
 *  @param: The process to be added to the end of the queue.
 *  @return: none
 */
void ready(pcb* process){
  if (process->pid == 0){return;}
  if (readyQueue == NULL){ //first time adding a process or if the queue is empty
	readyQueue = process;
	process->prev = NULL;
	process->next = NULL;
	process->state = READY;
	return;
  }
  
  //If the queue wasn't null, add process to the end of the queue
  pcb* tail = readyQueue;
  while (tail->next != NULL) {
	tail = tail->next;
  }
  tail->next = process;
  process->prev = tail; 
  process->next = NULL; 
  process->state = READY;
  return;
}

/*
 * Function:  readyRemoval 
 * --------------------
 * Checks readyqueue for the given process and removes the process.
 *
 *  @param: Process to be removed.
 *  @return: none
 */
void readyRemoval(pcb *process){
	if(process->state != READY){return;}
	if(process->prev != NULL){
		if(process->next != NULL){
			((pcb*)process->prev)->next = process->next;
			((pcb*)process->next)->prev = process->prev;
		} else {
			((pcb*)process->prev)->next = NULL;
		}
	} else {
		//kprintf("Removing head\n");
		if(process->next != NULL){
			readyQueue = process->next;
			readyQueue->prev = NULL;
		} else {
			//At head of queue with nothing in front
			readyQueue = NULL;
		}
	}
	process->next = NULL;
	process->prev = NULL;
}

/*
 * Function:  getRunningProc
 * --------------------
 * Returns a pointer to the currently running process
 *
 *  @param: none
 *  @return: Pointer to RUNNING process
 */
pcb* getRunningProc(void){
	return process;
}

/*
 * Function:  cleanUpWaiters
 * --------------------
 * Places all processes which are waiting on this process onto the readyQueue
 *
 *  @param:  Pointer to pcb
 *  @return: none
 */
void cleanUpWaiters(pcb* process){
	pcb *procToReady = process->waitingNext;
	while (procToReady != NULL){
		pcb *curr = procToReady;
		procToReady = curr->waitingNext;
		curr->waitingFor = 0;
		curr->waitingNext = NULL;
		ready(curr);
	}
	
	return;
}

/*
 * Function:  cleanup 
 * --------------------
 * Removes the process for any queues it may have been on and has it removed from
 *	the PCB table while freeing it's resources.
 *
 *  @param: Process to be destroyed.
 *  @return: none
 */
void cleanup(pcb* process){
  cleanUpWaiters(process);
  cleanUpSenders(process);
  cleanUpRecvers(process);
  //Remove the process from lists if it exists there
  switch (process->state){
  case SLEEPING:{
	  removeFromSleepList(process);
	  break;}
		
  case READY:{
	  readyRemoval(process);
	  break;}

  case BLOCKED:{

	  break;}
  case WAITING:{
	  //Remove self from the waiting list for a process
	  pcb *listStart = getPcb(process->waitingFor);
	  while(listStart->pid != process->pid){
		listStart = listStart->waitingNext;
	  }
	  listStart = process->waitingNext;	
	 
	  break;}
  }

  //remove the process from the pcb table and free resources
  removeProcess(process);

  return;
}


