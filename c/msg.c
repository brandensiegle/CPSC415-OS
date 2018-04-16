/* msg.c : messaging system 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <stdarg.h>
/* Your code goes here */

/*
 * Function:  send 
 * --------------------
 * Sends a message to another process.
 *
 *  @param:  The destination process PID
 *  @param:  Address of source buffer
 *  @param:  Length of the buffer
 *  @param:  Pointer to the PCB for the sender
 *  @return: The status of the send
 */
extern int send( PID_t dest_pid, void *buffer, int buffer_len, pcb *senderPCB){
  pcb *destPCB;
  int len;
  if (isCallingToSelf(&dest_pid, senderPCB)) {
    ready(senderPCB);
    return -2;
  }
  if (!isValidBuffer(buffer, buffer_len)){
    ready(senderPCB);
    return -3;
  }
  destPCB = getPcb(dest_pid);
  if (destPCB == (pcb*) SYSERR) { // check proc existence
    ready(senderPCB);
    return -1;
  }
  
  if (isProcReceiving(destPCB, senderPCB->pid)) { //isProcReceiving 
    len = runSend(destPCB, senderPCB);
  } else {
    senderPCB->state = BLOCKED;	
    len = blockSend(destPCB, senderPCB);
  }	
  
  return len;
}

/*
 * Function:  recv 
 * --------------------
 * Recieves a message from another process and returns the status of the
 *	message. PID of 0 signifies receive all.
 *
 *  @param:  The PID of the sending process
 *  @param:  Address of destination buffer
 *  @param:  Length of the buffer
 *  @param:  Pointer to the PCB of the receiving process
 *  @return: Status of the call
 */
extern int recv( PID_t *from_pid, void *buffer, int buffer_len, pcb *receiverPCB){
	pcb *fromPCB;
  int len;
  if (isCallingToSelf(from_pid, receiverPCB)) {
    ready(receiverPCB); 
    return -2;
  }
  if (!isValidBuffer(buffer, buffer_len)){
    ready(receiverPCB); 
    return -3;
  }
  fromPCB = getPcb(*from_pid);
  if ((*from_pid != 0) && (fromPCB == (pcb*) SYSERR)) { // check proc existence
    ready(receiverPCB); 
    return -1;
  }

  if (isProcSending(*from_pid, receiverPCB)) { 
    if (*from_pid == 0) {
      fromPCB = getFirstSender(receiverPCB);
    }
    len = runSend(receiverPCB, fromPCB);
  } else {
    receiverPCB->state = BLOCKED;
    len = blockRecv(*from_pid, receiverPCB);
  }
  return len;
}

/**
 * Function: isCallingToSelf
 * -----------------------
 * Checks if process is calling send/recv on itself
 *
 * @param: pid of process sending to/ receiving from
 * @param: pcb of process making the send/recv call
 * @return: TRUE if process calls send/recv on itself
            FALSE if not
 */
int isCallingToSelf(PID_t *target_pid, pcb *caller_pcb) {
  int isCallingSelf = FALSE;
  if (*target_pid == (PID_t) caller_pcb->pid) {
    isCallingSelf = TRUE;
  }
  return isCallingSelf;
  
}
/**
 * Function: isValidBuffer
 * -----------------------
 * Checks if buffer length is non-negative
 *
 * @param: buffer pointer
 * @param: buffer length
 * @return: TRUE buffer ptr and buffer length valid
            FALSE buffer ptr and/or buffer length are invalid
 */

int isValidBuffer(void *buffer, int buffer_len) {
  int isValid = TRUE;
  if (buffer_len < 0) {
    isValid = FALSE;
  }
  if (buffer == NULL) {
    isValid = FALSE;
  }
  return isValid;
}

/**
TODO if buffer, buflen stored in pcb, then no need for buffer, buffer_len
 * Function: runSend
 * -----------------------
 * Copy buffers, place send & recv to ready queue
 * Set appropriate return values
 *
 * @param: pcb of receiving end of send
 * @param: pcb of sending process
 * @return: TODO
 */
int runSend(pcb *dest_PCB, pcb *senderPCB) {
    // TODO set return values
  int len;
  len = transferBuffer(dest_PCB, senderPCB);
  rmBlockedSender(dest_PCB, senderPCB);
  rmBlockedRecver(dest_PCB);

  dest_PCB->ipcArgs = NULL;
  senderPCB->ipcArgs = NULL;
  dest_PCB->cpu_state.eax = len;
  senderPCB->cpu_state.eax = len;
  
  dest_PCB->next = NULL;
  dest_PCB->prev = NULL;

  senderPCB->next = NULL;
  senderPCB->prev = NULL;

  ready(senderPCB);
  ready(dest_PCB);
  return len;
}
/**
 * Function: blockSend
 * -----------------------
 * Block sender, place process onto sender queue, save parameters
 *
 * @param: pcb of receiving end of send
 * @param: pcb of sending process
 * @return: TODO
 */
int blockSend(pcb *dest_PCB, pcb *senderPCB) {
  addBlockedSender(dest_PCB, senderPCB);
  return 0;
}

/**
 * function: blockrecv
 * -----------------------
 * block receiver, place process onto receiver queue
 *
 * @param: pid of intended sender process
 * @param: pcb of receiving process
 * @return: todo
 */
int blockRecv(PID_t senderPid, pcb *recverPCB) {
  addBlockedRecver(senderPid, recverPCB);
  return 0;
}
/**
 * function: transferBuffer
 * -----------------------
 * copy buffer from sender to receiver
 * using the less of the two buffer lengths
 *
 * @param: pid of intended sender process
 * @param: pcb of receiving process
 * @return: TODO
 */

int transferBuffer(pcb *dest_PCB, pcb *senderPCB) {
  int cpLen;
  va_list *destVA_ptr = dest_PCB->ipcArgs;
  va_list *senderVA_ptr = senderPCB->ipcArgs;
  va_list destVA = *destVA_ptr;
  va_list senderVA = *senderVA_ptr;
  
  PID_t *recvPid = va_arg(destVA, PID_t*);
  char* recvBufPtr = va_arg(destVA, void*);
  int recvBufLen = va_arg(destVA, int);

  //PID_t sendPid = va_arg(senderVA, PID_t);
  char* sendBufPtr = va_arg(senderVA, void*);
  int sendBufLen = va_arg(senderVA, int);
  
  *recvPid = (PID_t) senderPCB->pid;
  cpLen = (recvBufLen > sendBufLen)? sendBufLen: recvBufLen;

  if (cpLen == 0) {
    return 0;
  } else {
    strncpy(recvBufPtr, sendBufPtr, cpLen);
  }
  
  return cpLen;
}


