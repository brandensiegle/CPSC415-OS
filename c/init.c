/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <tests.h>
#include <program.h>
extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/
/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
void initproc( void )				/* The beginning */
{
  
  char str[1024];
  int a = sizeof(str);
  int b = -17;
  int i; 

  kprintf( "\n\nCPSC 415, 2017W1 \n32 Bit Xeros -1.0.0 - even before beta \nLocated at: %x to %x\n", 
	   &entry, &end); 
  
  //kprintf("Some sample output to illustrate different types of printing\n\n");

  /* A busy wait to pause things on the screen, Change the value used 
     in the termination condition to control the pause
   */

  for (i = 0; i < 3000000; i++);

  /* Build a string to print) */
  sprintf(str, 
      "This is the number -17 when printed signed %d unsigned %u hex %x and a string %s.\n      Sample printing of 1024 in signed %d, unsigned %u and hex %x.\n\n",
	  b, b, b, "Hello", a, a, a);

  /* Print the string */

  kprintf("\n\nThe %dstring is: \"%s\"\n\nThe formula is %d + %d = %d.\n\n\n", 
	  a, str, a, b, a + b);

  for (i = 0; i < 1000000; i++);
  /* or just on its own */
  kprintf(str);

  /* Add your code below this line and before next comment */
  // Misc. information displayed for testing
  
  //Initializing Memory
  kmeminit();
  //Initializing Context
  contextinit();
  //Initializing PCB Table
  pcbTableInit(); 
  //Initialize ready and blocked queues
  initializeQueues();
  //Initialize the sleeping process deltaList
  sleepInit();
  //Initialize Device Table
  devTabInit();
  //Create the idle process
  create(idleproc,1024*50);

  
  //Create the root process
  int rootAllocSize = 256*1024; //using approximately .5MB size for the root process

  int isOK = create(mainProgram, rootAllocSize);
  if (isOK == SYSERR){ //Check to see if there was a problem creating root process
	kprintf("#INIT.c --> Failed to create initial process.\n");
	kprintf("\nHalting!"); 
	for(;;){}
  }

  //Call dispatcher
  dispatch();


  /* Add all of your code before this comment and after the previous comment */
  /* This code should never be reached after you are done */
  kprintf("\n\nWhen the kernel is working properly ");
  kprintf("this line should never be printed!\n");
  for(;;) ; /* loop forever */
}

/*
 * Function:  idleproc 
 * --------------------
 * Loops infinitely in user space
 *
 *  @param: none
 *  @return: none
 */
extern void idleproc( void ){
	//Idle procedure infinite loop
	for(int i=0;;i++){
		if(i%1000==0){
			//sysputs("IDLE\n");
			i = 0;
		}	
	}
	return;
}


