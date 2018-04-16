/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <tests.h>
#include <i386.h>
/* Your code goes here */

static PID_t rootPid;

/*
 * Function:  prodConsumers 
 * --------------------
 * Four of these processes are created for A2. The process listens
 *	for a message from the root process which tells it how long
 *	to sleep for.
 *
 *
 *  @param: none
 *  @return: none
 */
void testGCT_pc(void){
	char printBuf[256];
	char buf[8];
	int sleepTime;

	//Print message indicating alive
	sprintf(printBuf,"Process %d | I am alive.\n", sysgetpid());
	sysputs(printBuf);

	//sleeps for 5 seconds
	syssleep(5000);

	sprintf(printBuf,"Process %d | I am awake.\n", sysgetpid());
	sysputs(printBuf);

	//Recieve the amount of time to sleep for
	int err = sysrecv(&rootPid, buf, 8);
	if(err < 0){
		//If there is an error, print the error code and terminate.
		sprintf(printBuf,"Process %d | Error recieved. Error code %d.\n",sysgetpid(), err);
		sysputs(printBuf);
		return;
	}

	//If there is no error, get time
	sleepTime = atoi(buf);

	//display what was recieved.
	sprintf(printBuf,"Process %d | Message recieved. Sleeping for %dms.\n",sysgetpid(), sleepTime);
	sysputs(printBuf);

	//Sleep for recieved time.
	syssleep(sleepTime);

	//Alert that woken up and ending
	sprintf(printBuf,"Process %d | Sleeping has stopped. Running off end of process code.\n",sysgetpid());
	sysputs(printBuf);
}


/*
 * Function:  root 
 * --------------------
 * Stars 4 of the A2 processes and tells each to sleep for a
 *	different amount of time. Then it will try to read from
 *	two differences that are expected to fail.
 *
 *
 *  @param: none
 *  @return: none
 */
void testGCT_root(void){
  char printBuf[256];
  PID_t pids[4];
  char  sendBuf[8];
  char  recvBuf[8];

  //*   Updated Consumer Producer   *//
  rootPid = sysgetpid();

  //Print message indicating alive
  sprintf(printBuf,"Process %d | Root process alive.\n", sysgetpid());
  sysputs(printBuf);

  //Creates 4 processes while indicating so
  for(int i=0;i<4;i++){
    pids[i] = syscreate(testGCT_pc ,1024*5);
    sprintf(printBuf,"Process %d | Starting process with PID: %d\n",sysgetpid(),pids[i]);
    sysputs(printBuf);
  }


  //sleep for 4 seconds
  syssleep(4000);

  //sends messages
  //@@@@To third proc -- 10000
  sprintf(sendBuf, "10000");
  syssend(pids[2], &sendBuf, 8);

  //@@@@To second proc -- 7000
  sprintf(sendBuf, "7000");
  syssend(pids[1], &sendBuf, 8);

  //@@@@To first proc -- 20000
  sprintf(sendBuf, "%d", 20000);
  syssend(pids[0], &sendBuf, 8);

  //@@@@To fourth proc -- 27000
  sprintf(sendBuf, "%d", 27000);
  syssend(pids[3], &sendBuf, 8);

  processStatuses psTab;
  int procs;

  procs = sysgetcputimes(&psTab);

  sprintf(printBuf, "sysgetcputimes %d\n", procs);
  sysputs(printBuf);
  for (int j=0; j<=procs; j++) {
    sprintf(printBuf, "%4d  %4d   %10d\n", psTab.pid[j], psTab.status[j], \
      psTab.cpuTime[j]);
    sysputs(printBuf);
  }

  //Try to read from 4th process
  int err = sysrecv(&pids[3], recvBuf, 8);

  //Print error code
  if(err < 0){
    //If there is an error, print the error code and terminate.
    sprintf(printBuf,"Process %d | Error recieved. Error code %d.\n",sysgetpid(), err);
    sysputs(printBuf);
  }

  //Try to send to 3rd process
  sprintf(sendBuf, "Hello!!!");
  err = syssend(pids[2], &sendBuf, 8);

  //Print error code
  if(err < 0){
    //If there is an error, print the error code and terminate.
    sprintf(printBuf,"Process %d | Error recieved. Error code %d.\n",sysgetpid(), err);
    sysputs(printBuf);
  }

  procs = sysgetcputimes(&psTab);

  sprintf(printBuf, "sysgetcputimes %d\n", procs);
  sysputs(printBuf);
  for (int j=0; j<=procs; j++) {
    sprintf(printBuf, "%4d  %4d   %10d\n", psTab.pid[j], psTab.status[j], \
      psTab.cpuTime[j]);
    sysputs(printBuf);
  }

  procs = sysgetcputimes((processStatuses *) HOLESTART + 16);
  sprintf(printBuf, "sysgetcputimes HOL %d\n", procs);
  sysputs(printBuf);

  procs = sysgetcputimes((processStatuses  *) 0x400001);
  sprintf(printBuf, "sysgetcputimes past main mem  %d\n", procs);
  sysputs(printBuf);

  //call syskill on itself
  //syskill(sysgetpid());


  //This infinite loop should not be reached as long as the process stopped
  kprintf("#USER.c --> ERROR: Should have terminated!\n");
  for(;;) ;
}

