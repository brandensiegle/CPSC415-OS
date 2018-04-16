/* program.c : test programs
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <program.h>

extern int pmVal;
extern int a_progPid;

/** ps **/ 
void  prog_ps(void) {
	processStatuses psTab;
	char printBuf[256];
  	int procs = sysgetcputimes(&psTab);

	//PRINT RESULTS
	sprintf(printBuf, "%4s %4s  %10s\n", "PID", "Stat", "CPU Time");
	sysputs(printBuf);
	for(int i =0;i<procs+1;i++){
		sprintf(printBuf, "%4d %4d  %10d\n", psTab.pid[i], psTab.status[i], psTab.cpuTime[i]);
		sysputs(printBuf);
	}	
	
}

/** k **/
// return -1 if process does not exist
int   prog_k(int PID) {
	int result = syskill(PID,31);
	if(result == -512){
		sysputs("No such process\n");
		return -1;
	} else {
		return result;
	}	
}

/** a **/
// bkgd is either TRUE of FALSE
//  corresponds to whether there is an '&'
void alarmHandler(void){
	sysputs("ALARM ALARM ALARM\n");
	syssighandler(15, NULL, NULL);
}

void  prog_a(void) {
	int ticks = pmVal;
	int requesterPid = a_progPid;
	int milliseconds = TIME_SLICE * ticks;
	syssleep(milliseconds);
	syskill(requesterPid, 15);
}

/** t **/
void  prog_t(void) {
	for(;;){
		syssleep(10000);
		sysputs("\nT\n> ");
	}
}

