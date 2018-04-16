/* msgQueues.c : msg functions pertaining to pcb
*/

#include <xeroskernel.h>
#include <xeroslib.h>

/*
 * Function:  isProcReceiving
 * --------------------
 *  Checks if target process of syssend is receiving to the caller
 *  If target process is receiving to 0 (recvAll),
 *  the function will return TRUE
 *
 *  @param:  PCB of caller of syssend
 *  @param:  PID of target of syssend
 *  @return: TRUE if process is receiving, else FALSE
 */
int isProcReceiving(pcb *recverPCB, PID_t senderPid) {
  if (((PID_t) recverPCB->target_of_recv == senderPid) ||\
    ((PID_t) recverPCB->target_of_recv == 0)){
    return TRUE;
  } else {
    return FALSE;
  }
}

/*
 * Function:  addBlockedSender
 * --------------------
 *  Add a process to sender queue of another process
 *  @param:  PCB of process to be added to sender queue
 *  @param:  PCB of owner of sender queue
 *  @return: OK indicating success
 */
int addBlockedSender(pcb *recverPCB, pcb *senderPCB) {
  pcb *node = (pcb *) recverPCB->sender_queue;
  if (node == NULL) {
    recverPCB->sender_queue = senderPCB;
    return OK;
  }
  while(node->next != NULL) {
    node = (pcb *) node->next;
  }
  node->next = senderPCB;
  senderPCB->next = NULL;
  return OK;
}

/*
 * Function:  rmBlockedSender
 * --------------------
 *  Remove caller of syssend from the sender queue of a process
 *  @param:  PCB of process to be removed from sender queue
 *  @param:  PCB of owner of sender queue
 *  @return: OK indicating success
 */

int rmBlockedSender(pcb *recverPCB, pcb *senderPCB) {
  pcb *node = (pcb *) recverPCB->sender_queue;
  if (node == NULL) {
    return FAILED;
  }
  if (node == senderPCB) {
    recverPCB->sender_queue = senderPCB->next;
    senderPCB->next = NULL;
    return OK;
  }
  while(node->next != NULL) {
    if ((pcb *) node->next == senderPCB) {
      node->next = senderPCB->next;
      senderPCB->next = NULL;
      return OK;
    }
    node = (pcb *) node->next;
  }
  return FAILED;
}

/*
 * Function:  isProcSending
 * --------------------
 *  Check if a process is sending to another process
 *  @param:  PID of process doing the sending
 *  @param:  PCB of process that is the target of a send
 *  @return: TRUE if the process is sending to the receiver, 
 *            else, FALSE
 */

int isProcSending(PID_t senderPid, pcb *recverPCB) {
  int isSending = FALSE;
  pcb* sender = (pcb *) recverPCB->sender_queue;
  if ((senderPid == 0) && (sender != NULL)) {
    return TRUE;
  }
  while (sender != NULL) {
    if (sender->pid == senderPid) {
      isSending = TRUE;
      break;
    }
    sender = (pcb *) sender->next;
  }
  return isSending;
}

/*
 * Function:  getFirstSender
 * --------------------
 *  Get PCB of first process on sender queue
 *  If the sender queue is empty, NULL is returned
 *  @param:  PCB of owner of sender queue
 *  @return: PCB pointer of sender process or NULL
 */

pcb *getFirstSender(pcb *recverPCB) {
  return (pcb *) recverPCB->sender_queue;
}

/*
 * Function:  addBlockedRecver
 * --------------------
 *  Block process and indicate who it is receiving to
 *  @param:  PID of process it is receiving to
 *  @param:  PCB of process to be blocked
 *  @return: OK indicating success
 */
int addBlockedRecver(PID_t senderPid, pcb *recverPCB) {
  recverPCB->target_of_recv = senderPid;
  return OK;
}

/*
 * Function:  rmBlockedRecver
 * --------------------
 *  Reset process of flags that indicate it is blocked for receiving
 *  target_of_recv flag of PCB is set to it's own PID
 *  @param:  PCB of process to be unblocked
 *  @return: OK indicating success
 */
int rmBlockedRecver(pcb *recverPCB) {
  recverPCB->target_of_recv = recverPCB->pid;
  return OK;
}

/*
 * Function:  printSenderQueue
 * --------------------
 *  Print sender queue of process
 *  @param:  PCB of receiving process
 */
void printSenderQueue(pcb *recverPCB) {
  //kprintf("K:ipc:pr %d: ", recverPCB->pid);
  pcb *node = (pcb *) recverPCB->sender_queue;
  while (node != NULL) {
    //kprintf("[%d] %d, ", node->pid, node);
    node = (pcb *) node->next;
  }
  //kprintf("/\n");
}

/*
 * Function:  cleanUpSenders
 * --------------------
 *  Unblock all processes that are sending to this process
 *  set return value of unblocked process to -1
 *
 *  @param:  PCB of process receiving the sends
 *  @return: OK indicating success
 */
int cleanUpSenders(pcb *cleanUpPcb) {
  pcb *node = (pcb *) cleanUpPcb->sender_queue;
  while (node != NULL) {
    //node = (pcb *) node->next;
    node = cleanUpSendersHelper(cleanUpPcb, node);
  }
  return OK;
}

/*
 * Function:  cleanUpSenderHelper
 * --------------------
 *  Remove process from sender queue, unblock process
 *  Set return value to -1
 *
 *  @param:  PCB of process receiving the sends
 *  @param:  PCB of process doing send
 *  @return: next process on sender queue of cleanUpPcb
 */
pcb *cleanUpSendersHelper(pcb *cleanUpPcb, pcb *senderPcb) {
  pcb *node = (pcb *) senderPcb->next;
  // remove sender
  rmBlockedSender(cleanUpPcb, senderPcb);
  // set return address
  senderPcb->cpu_state.eax = -1;
  // put on ready queue
  ready(senderPcb);
  return node;
}

/*
 * Function:  cleanUpRecvers
 * --------------------
 *  Unblock all processes that are receving to this process
 *  set return value of unblocked process to -1
 *
 *  @param:  PCB of process getting the receives
 *  @return: OK indicating success
 */
int cleanUpRecvers(pcb *cleanUpPcb) {
  PID_t recvers[PCBTABLE_SIZE+1];
  pcb *recverPCB;
  getAllRecvers((PID_t) cleanUpPcb->pid, recvers);
  
  int i = 0;
  //kprintf("K:ipc:cl R %d\n", recvers[0]);
  while(recvers[i] != NULL) {
    recverPCB = getPcb(recvers[i]);
    if (recverPCB != (pcb*) SYSERR) {
      //kprintf("K:ipc:cl[%d] R:%d \n", cleanUpPcb->pid, recverPCB->pid);
      cleanUpRecversHelper(recverPCB);
    } else {
      //kprintf("K:ipc:cleanup: receiver of proc %d has invalid pid %d\n",
      //cleanUpPcb->pid, recvers[i]);
    }
    i++;
  }
  //printSenderQueue(cleanUpPcb);
  //printReadyQueue();
  return OK;
}

/*
 * Function:  cleanUpRecversHelper
 * --------------------
 *  Remove process from blocked receive
 *  set return value to -1, unblock process
 *  @param:  PCB of process to be unblocked
 *  @return: OK indicating success
 */
int cleanUpRecversHelper(pcb *recverPCB) {
  // clear target_of_receiver flag
  rmBlockedRecver(recverPCB); 
  // set return value
  recverPCB->cpu_state.eax = -1;
  // put on ready queue
  ready(recverPCB);
  return OK;
}
