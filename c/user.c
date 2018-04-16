/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>
/* Your code goes here */

//static PID_t rootPid;

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
void prodConsumers(void){/*
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
	sysputs(printBuf);*/
}

void waiter(void){
	sysputs("Waiting on 2\n");
	syswait(2);
	sysputs("2 killed\n");
}

void runner(void){
	for(;;){}
}



void Handler29(void){
	sysputs("29\n");
}

void Handler28(void){
	sysputs("28\n");
}

void Handler27(void){
	sysputs("27\n");
}

void Handler26(void){
	sysputs("26\n");
}

void Handler25(void){
	sysputs("25\n");
}

void Handler24(void){
	sysputs("Handler\n");
}


void sigTest(void){
	/*void *oldH;
	char printBuf[256];
	char sendBuf[8];
	
	
	int result = syssighandler(31,&Handler29,&oldH);
	sprintf(printBuf,"Result of handler install: %d | Expected: -1\n", result);
	sysputs(printBuf);

	result = syssighandler(32,&Handler28,&oldH);
	sprintf(printBuf,"Result of handler install: %d | Expected: -1\n", result);
	sysputs(printBuf);

	result = syssighandler(-1,&Handler27,&oldH);
	sprintf(printBuf,"Result of handler install: %d | Expected: -1\n", result);
	sysputs(printBuf);

	result = syssighandler(5,&Handler26,&oldH);
	sprintf(printBuf,"Result of handler install: %d | Expected: 0\n", result);
	sysputs(printBuf);

	result = syssighandler(10,&Handler25,&oldH);
	sprintf(printBuf,"Result of handler install: %d | Expected: 0\n", result);
	sysputs(printBuf);

	int result = syssighandler(10,&Handler24,&oldH);
	sprintf(printBuf,"Result of handler install: %d | Expected: 0\n", result);
	sysputs(printBuf);
	
	result = syssend(1, &sendBuf, 8);
	sprintf(printBuf,"Result: %d | Expected: -99\n", result);
	sysputs(printBuf);

	for(;;){}*/
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
void root(void){/*
char printBuf[256];

sysputs("syswait test\n");

int toKill = syscreate(&runner, 1024*5);
syscreate(&waiter, 1024*5);
syssleep(10000);
syskill(toKill, 31);

for(;;){}


sysputs("Creating sigTest\n");
int pid = syscreate(sigTest,1024*5); //PID 2

syssleep(5000);
sysputs("Set 1\n");

syskill(pid,10); //Expected: 0 ~~ Signal success (since handler for 29 is installed)
for(;;){}
syskill(pid,27); 
syskill(pid,29); 
syskill(pid,29);
syskill(pid,28);

syssleep(10000);
sysputs("Set 2\n");

syskill(pid,25); //Expected: 0 ~~ Signal success (since handler for 29 is installed)
syskill(pid,27); 
syskill(pid,29); 
syskill(pid,31);
syskill(pid,28);

for(;;){}


int result = syskill(0, 29); //Expected: -512 ~~ Process does not exist
int expected = -512; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(-1,29); //Expected: -512 ~~ Process does not exist
expected = -512; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(32,29); //Expected: -512 ~~ Process does not exist
expected = -512; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(20,29); //Expected: -512 ~~ Process does not exist
expected = -512; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(pid,0); //Expected: 0 ~~ Signal ignored (since no handler for 0 is installed)
expected = 0; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(pid,5); //Expected: 0 ~~ Signal ignored (since no handler for 5 is installed)
expected = 0; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(pid,-1); //Expected: -561 ~~ Invalid signal number
expected = -561; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(pid,32); //Expected: -561 ~~ Invalid signal number
expected = -561; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

result = syskill(pid,29); //Expected: 0 ~~ Signal success (since handler for 29 is installed)
result = syskill(pid,29); 
result = syskill(pid,27); 
result = syskill(pid,28);
result = syskill(pid,28);
result = syskill(pid,28);
result = syskill(pid,31); 
result = syskill(pid,28);
expected = 0; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);

for(;;){}

syssleep(1000);

result = syskill(pid,29); 
result = syskill(pid,29); 
expected = -512; if(result != expected){sysputs("~X~");}
sprintf(printBuf,"Result of signal: %d | Expected: %d\n", result, expected);
sysputs(printBuf);


for(;;){}



int a = syscreate(waiter,1024*5);
syssleep(1000);

syscreate(runner,1024*5);
syssleep(1000);

syscreate(waiter,1024*5);
syssleep(1000);

syscreate(waiter,1024*5);
syssleep(1000);

sysputs("Sleeping root... Zzzzz....\n");
syssleep(1000);

sysputs("Root awake!\n");
syskill(3,31);


for(;;){}
char *wrBuf = NULL;
sprintf(wrBuf,"%d",a);

sysputs(wrBuf);














//char printBuf[256];
PID_t pids[4];
char  sendBuf[8];
char  recvBuf[8];

//   Updated Consumer Producer   //
rootPid = sysgetpid();

//Print message indicating alive
sprintf(printBuf,"Process %d | Root process alive.\n", sysgetpid());
sysputs(printBuf);

//Creates 4 processes while indicating so
for(int i=0;i<4;i++){
	pids[i] = syscreate(prodConsumers,1024*5);
	sprintf(printBuf,"Process %d | Starting process with PID: %d\n",sysgetpid(),pids[i]);
	sysputs(printBuf);
}

  //   Updated Consumer Producer   //
  rootPid = sysgetpid();

  //Print message indicating alive
  sprintf(printBuf,"Process %d | Root process alive.\n", sysgetpid());
  sysputs(printBuf);

  //Creates 4 processes while indicating so
  for(int i=0;i<4;i++){
    pids[i] = syscreate(prodConsumers,1024*5);
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

  //call syskill on itself
  syskill(2,31);


  //This infinite loop should not be reached as long as the process stopped
  kprintf("#USER.c --> ERROR: Should have terminated!\n");
  for(;;) ;*/
}


