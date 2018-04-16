/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>
#include <stdint.h>

/* Your code goes here */

static unsigned int call_number;	/* Location in IDT */
static unsigned long ret_val;		/* Return value of interrupt */
static va_list args;			/* Arguments to pass to ISR */

/*
 * Function:  syscall 
 * --------------------
 * Handles jumping to the InteruptServiceRoutine based on the
 *	call number specified. It will store a pointer to any
 *	other arguments passed in using register %edx. If there
 *	is a return value in %eax then it will be passed back.
 *
 *  @param: the integer value of the entry in the interrupt table
 *  @param: (multiple) can be any number and types to be passed on
 *	    to the interrupt handler.
 *  @return: A value passed back from the interrupt handler.
 */
extern int syscall(int call, ...){
	
  //gather all of the arguments other than the call argument
  va_start(args,call);
  
  call_number = (int32_t) call;
  //The return value that will be stored in %eax after execution
  ret_val = (unsigned long) NULL;
  //Assembly code that will store values in registers and jump to
  //the correct place in the interrupt table
  __asm __volatile( " \
      movl call_number, %%eax      \n\
      movl args, %%edx	\n\
      int $41 \n\
      movl %%eax, ret_val    \n\
      "
      :
      :
      :"%eax"
  );

  //Freeing memory held by the stored arguments
  va_end (args);
	return ret_val;
}

/*
 * Function:  syscreate 
 * --------------------
 * Makes a system call to create a new process with a given size
 *	of memory. If the creation is successful, it will return
 *	a OK, if it is unsuccessful it will return a SYSERR
 *
 *  @param: The address to the start of the function's code
 *  @param: The size of the memory to allocate for this process.
 *  @return: A value informing of success or failure in creating the process
 */
extern unsigned int syscreate( void (*func)(void), int size){
	return syscall(CREATE, func, size);
}

/*
 * Function:  sysyield 
 * --------------------
 * Causes the current process to YIELD so that another process may
 *	start running.
 *
 *  @param: none
 *  @return: none
 */
extern void sysyield( void ) {
	syscall(YIELD);
	return;
}

/*
 * Function:  sysstop 
 * --------------------
 * Causes the current process to stop and release all resources that were
 *	being used by the process.
 *
 *  @param: none
 *  @return: none
 */
extern void sysstop( void ){
	syscall(STOP);
	return;
}

/*
 * Function:  sysputs 
 * --------------------
 * Prints the supplied string to console. Won't print beyond a NULL value.
 *
 *  @param: NULL terminated string to print
 *  @return: none
 */
extern void sysputs( char *str ){
	syscall(PUTS, str);
	return;
}

/*
 * Function:  sysgetpid 
 * --------------------
 * Returns the pid of the currently running process.
 *
 *  @param: none
 *  @return: pid of current process
 */
extern PID_t sysgetpid( void ){
	return syscall(GETPID);
}

/*
 * Function:  syskill 
 * --------------------
 * Function used to send signals to processes.
 *
 *  @param:  Process ID of the process to signal
 *  @param:  Signal number
 *  @return: Status of operation
 */
extern int syskill( int pid , int signalNumber){
	return syscall(KILL, pid, signalNumber);
}

/*
 * Function:  syssend 
 * --------------------
 * Sends message to another process.
 * If the other process is not receiving, the current process 
 * will be blocked until a receive call.
 *
 *  @param: PID of the receiving process.
 *  @param: Pointer of the send buffer
 *  @param: Length of message
 *  @return: Length of message sent
 */
extern int syssend( PID_t dest_pid, void *buffer, int buffer_len ){
	return syscall(SEND, dest_pid, buffer, buffer_len);
}

/*
 * Function:  sysrecv 
 * --------------------
 * Receives message from another process.
 * If the receiving process did not make a send, the current process
 * will wait by blocking.
 * 
 * Setting the from PID to 0 will cause the process to do a receive all.
 *
 *  @param: Pointer to PID of sending process. If the pointer points to PID: 0,
 *      The process will receive all. Upon return, the pointer will be set to 
 *      the PID of the sending process
 *  @param: Pointer to the receiving buffer
 *  @param: Length of receiving buffer
 *  @return: Length of message received
 */
extern int sysrecv( PID_t *from_pid, void *buffer, int buffer_len ){
	return syscall(RECV, from_pid, buffer, buffer_len);
}

/*
 * Function:  syssleep 
 * --------------------
 * Places the current process into a sleeping state. It will be woken
 *	up after the time has passed or another process signals it.
 *
 *  @param:  Number of milliseconds to sleep for
 *  @return: Number of remaining milliseconds when woken (0 if never signaled)
 */
extern unsigned int syssleep( unsigned int milliseconds ){
	return syscall(SLEEP, milliseconds);
}

/*
 * Function:  sysgetcputimes
 * --------------------
 * Returns pid, current process state, and number of milliseconds charged
 *    for each process.
 *
 *  @param:  Pointer to a processStatuses struct to be modified.
 *  @return: Value of last slot used
 *           -1 if the address is in the hole
 *           -2 if the address goes beyond main memory
 */
extern int sysgetcputimes(processStatuses *ps) {
  return syscall(GETCPUTIMES, ps);
}

/*
 * Function: sysopen
 * --------------------
 * Opens device and returns file descriptor
 *
 * @param: Major device number
 * @return: Index of fdt, or -1 if open failed
 *
 */
extern int sysopen(int device_no) {
  return syscall(DV_OPEN, device_no);
}
/*
 * Function: sysclose
 * --------------------
 * Closes device 
 *
 * @param: File descriptor
 * @return: 0 if close is successful, otherwise -1
 *
 */
extern int sysclose(int fd) {
  return syscall(DV_CLOSE, fd);
}
/*
 * Function: syswrite
 * --------------------
 * Writes to device 
 *
 * @param: File descriptor
 * @param: Buffer for write
 * @paran: Number of bytes to write
 * @return: Number of bytes actually written, 
 *          -1 if error occured
 *
 */
extern int syswrite(int fd, void* buffer, int buffer_len) {
  return syscall(DV_WRITE, fd, buffer, buffer_len);
}
/*
 * Function: sysread
 * --------------------
 * Reads from device
 *
 * @param: File descriptor
 * @param: Buffer for read
 * @param: Number of bytes for read
 * @return:
 *
 */
extern int sysread(int fd, void* buffer, int buffer_len) {
  return syscall(DV_READ, fd, buffer, buffer_len); 
}
/*
 * Function: sysioctl
 * --------------------
 * Sends ioctl call to device
 *
 * @param: File descriptor
 * @param: Control command number
 * @param: (multiple) parameters for command
 * @return: 0 if successful, -1 otherwise
 *
 */
extern int sysioctl(int fd, unsigned long command, ...) {
  va_list ioctl_args;
  int result;
  va_start(ioctl_args, command);
  result = syscall(DV_IOCTL, fd, command, &ioctl_args);
  va_end(ioctl_args);
  return result; 
}
/*
 * Function:  syssighandler 
 * --------------------
 * Installs the provided signal handler for the running process.
 *	Valid signals to set handlers for are 0~30; 31 is reserved.
 *
 *  @param:  signal number to set a handler for.
 *  @param:  pointer to handler function
 *  @param:  pointer to location to store old handler address
 *  @return: status of operation
 */
extern int syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *)){
	return syscall(SIGHANDLE, signal, newhandler, oldHandler);
}

/*
 * Function:  syswait 
 * --------------------
 * Has the process wait for the specified process to stop.
 *
 *  @param:  Process ID of the process to wait on
 *  @return: Status of operation
 */
extern int syswait(int PID){
	return syscall(WAIT, PID);
}

/*
 * Function:  syssigreturn 
 * --------------------
 * ONLY CALLED BY SIGTRAMP CODE
 *	
 * Sets the current stack pointer to the one provided.
 *
 *  @param:  Pointer to new process stack location.
 *  @return: none
 */
extern void syssigreturn(void *old_sp){
	syscall(SIGRETURN, old_sp);
	return;
}
