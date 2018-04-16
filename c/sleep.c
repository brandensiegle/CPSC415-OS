/* sleep.c : sleep device 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */
static pcb* sleepList;		/* List of sleeping processes	*/




/*
 * Function:  sleepInit 
 * --------------------
 * Initialize the sleepList
 *
 *  @param: None
 *  @return: None
 */
extern void sleepInit(void){
	sleepList = NULL;
}
/*
 * Function:  addToSleepList 
 * --------------------
 * Adds the process to the sleepList with a value for the given time.
 *
 *  @param: Process to add to sleepList
 *  @param: Number of milliseconds to sleep for
 *  @return: None
 */
void addToSleepList(pcb* process,unsigned int time){
	if(sleepList == NULL){ //case when list is empty
		process->sleepTime = time;
		process->sleepNext = NULL;
		process->sleepPrev = NULL;	
		sleepList = process;
	} else {
		process->sleepTime = time;
		pcb* current = sleepList;

		while(current->sleepTime <= process->sleepTime){		//while we haven't found our place
			//lower the process' timer relative to the place on the list
			process->sleepTime = process->sleepTime - current->sleepTime;	
			if(current->sleepNext == NULL){		//if at the end
				current->sleepNext = process;
				process->sleepPrev = current;
				process->sleepNext = NULL;
				return;
			}
			current = current->sleepNext;
		}

		//Check to see if we are goint to be inserted before the HEAD
		if(current->sleepPrev != NULL){
			//insert before current
			process->sleepPrev = current->sleepPrev;
			process->sleepNext = current;
		
			((pcb*) current->sleepPrev)->sleepNext = process;
			current->sleepPrev = process;

				
		} else {
			current->sleepPrev = process;
			process->sleepNext = current;
			sleepList = process;
		}
		//decrement the time of the next sleeping process
		if(current != NULL){
			current->sleepTime = current->sleepTime-process->sleepTime;
		}	
	}
	return;
}


/*
 * Function:  sleep 
 * --------------------
 * Places the current running process into a SLEEPING state for a
 *	given time.
 *
 *  @param: Number of milliseconds to sleep for
 *  @return: Integer indicating success or failure
 */
extern int sleep(unsigned int milliseconds){
	pcb* process = getRunningProc();

	if(milliseconds == 0){ //if the amount of time was 0; just put on ready right away
		process->cpu_state.eax = 0;
		ready(process);
		return OK;
	}
	process->state = SLEEPING;
	addToSleepList(process,milliseconds);
	
	return OK;	//This isn't the value passed back to the sleeping function
}




/*
 * Function:  wake 
 * --------------------
 * Pops the current process from the sleep list, sets
 *	it's return value to be the remaining time, and
 *	places it on the readyQueue.
 *
 *  @param: The current process to wake
 *  @return: none
 */
void wake(pcb* process){
	//keep it to remove that from the next item
	pcb* popped = sleepList;			//pop the process from the sleepList
	sleepList = popped->sleepNext;			//update the front of the sleepList
			
	popped->cpu_state.eax = popped->sleepTime;	//set the return value to be 0
	popped->sleepNext = NULL;
	popped->sleepPrev = NULL;
	ready(popped);					//add the popped process to the readyQueue
	return;
}

/*
 * Function:  tick 
 * --------------------
 * Everytime tick is called, the sleepTime for the first process
 *	on the sleepList will be decremented. When it is less
 *	than or equal to one, it will be placed on the ready
 *	queue and the following processes will be checked.
 *
 *  @param: none
 *  @return: none
 */
extern void tick(void){
	//Do nothing for empty list
	if(sleepList == NULL){return;}

	//Decrement the first process' time by one time slice
	sleepList->sleepTime = sleepList->sleepTime-TIME_SLICE;
	
	//Check to see if the first item is ready to be woken
	while(sleepList->sleepTime <= 0 && sleepList != NULL){
		if(sleepList->sleepTime == 0){
			wake(sleepList);
		} else if (sleepList->sleepTime < 0){
			int timeDiff = (-1)*(sleepList->sleepTime);	//We removed too much time from the process
			sleepList->sleepTime = 0;
			wake(sleepList);
			sleepList->sleepTime = sleepList->sleepTime-timeDiff;	//remove the extra time	
		}
	}
	return;
}

/*
 * Function:  removeFromSleepList 
 * --------------------
 * Remove a given process from the sleepList and adjust
 *	the next process' time accordingly.
 *
 *  @param: Process to remove from list
 *  @return: Status of operation
 */
extern int removeFromSleepList(pcb* process){

	if(process->state != SLEEPING){kprintf("FAIL\n");return FAILED;}

	if(process->sleepPrev != NULL){
		if(process->sleepNext == NULL){
			((pcb *)process->sleepPrev)->sleepNext = process->sleepNext;
		} else {
			((pcb *)process->sleepNext)->sleepTime = ((pcb *)process->sleepNext)->sleepTime + process->sleepTime;
			((pcb *)process->sleepPrev)->sleepNext = process->sleepNext;
			((pcb *)process->sleepNext)->sleepPrev = process->sleepPrev;
		}
	} else {
		if(process->sleepNext == NULL){
			sleepList = NULL;
		} else {
			((pcb *)process->sleepNext)->sleepTime = ((pcb *)process->sleepNext)->sleepTime + process->sleepTime;
			sleepList = process->sleepNext;
		}
	}

	process->sleepPrev = NULL;
	process->sleepNext = NULL;
	process->state = NULL;
	process->cpu_state.eax = process->sleepTime;

	return OK;
}
