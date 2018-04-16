/* shell.c : shell program
 */

#include<xeroskernel.h>
#include<xeroslib.h>
#include<program.h>

int pmVal;
int a_progPid;

void shell( void ) {
  char pBuf[64];
  int fd;
  int r;
  char cmdBuf[32];
  int buflen = 32;
  int bkgd;
  int pid;

  pid = sysgetpid();

  fd = sysopen(KBD1);
  while (1) {
    sprintf(pBuf, "> ");
    sysputs(pBuf);
    r = sysread(fd, cmdBuf, buflen);
    if (r > 0) {
      bkgd = cropInput(cmdBuf, &r);
      handleShellCmd(pid, fd, cmdBuf, r, bkgd);
      continue;
    } else {
      if (r == 0) {
        sysclose(fd);
        sysstop();
      } else {
        sprintf(pBuf, "ERROR: sysread error %d\n", r);
        sysputs(pBuf);
      }
    }
  }
}

void handleShellCmd(int pid, int fd, char *buf, int len, int bkgd) {
  char cmdBuf[32]; char pmBuf[32];
  int  cmdLen; int pmLen;
  int  known;
  int  funcPid;
  a_progPid = pid;
  known = FALSE;
  parseInput(buf, len, cmdBuf, pmBuf, &cmdLen, &pmLen);
  
  char pBuf[64];


  if (strcmp(cmdBuf, "ps") == 0) {
    known = TRUE;
    prog_ps();
  }
  if (strcmp(cmdBuf, "ex") == 0) {
    known = TRUE;
    sysclose(fd);
    sysstop();
  }
  if (strcmp(cmdBuf, "k") == 0) {
    known = TRUE;
    pmVal = atoi(pmBuf);
    if (pmVal == pid) {
      sysclose(fd);
      sysstop();
    } else {
      prog_k(pmVal);
    }
  }
  if (strcmp(cmdBuf, "a") == 0) {
    known = TRUE;
    pmVal = atoi(pmBuf);
    void *oldH;
    syssighandler(15,(void *) &alarmHandler,(void*) &oldH);

    /*void wrapper( void ) {
	sysputs("In wrapper\n");
      prog_a(pmVal, pid);
      syssleep(3000);
      kprintf("!!\n");
    }*/
    funcPid = syscreate(&prog_a, 4*1024);

    if (!bkgd) {
      syswait(funcPid);
    }
  }
  if (strcmp(cmdBuf, "t") == 0) {
    known = TRUE;
    syscreate(prog_t, 4*1024);
  }
  if (known == FALSE) {
    sprintf(pBuf, "Command not found\n");
    sysputs(pBuf);
  }
}

// Parse the substring from start to first white space as "command"
// Then find the first non-whitespace after the first whitespace.
// That to the end is "param"
void parseInput(char *buf, int len, char *cmdBuf, char *pmBuf, int *cmdLen, int *pmLen) {
  int cmd_end; int pm_start;
  int cmd_len; int pm_len;
  // find end of command
  cmd_end = 0;
  while ((cmd_end<len) &&  (!isWhiteSpace(&buf[cmd_end]))) {
    cmd_end++;
  }
  
  // find start of param
  pm_start = cmd_end;
  while((pm_start<len) && (isWhiteSpace(&buf[pm_start]))) {
    pm_start++;
  }
  
  // copy command
  cmd_len = calculateLen(0, cmd_end-1);
  strncpy(cmdBuf, buf, cmd_len);
  cmdBuf[cmd_len] = '\0';
  *cmdLen = cmd_len;

  // copy param
  pm_len = calculateLen(pm_start, len-1);
  strncpy(pmBuf, &buf[pm_start], pm_len);
  pmBuf[pm_len] = '\0';
  *pmLen = pm_len;
  return;
}
