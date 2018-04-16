#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <tests.h>

static PID_t senderPid;
static PID_t recverPid;
static PID_t pids[5];

void initPids(void) {
  int n = sizeof(pids) / sizeof(pids[0]);
  for (int i=0; i<n; i++) {
    pids[i] = -1;
  }
}

void testIpc1(void) {
  senderPid = -1;
  recverPid = -1;
  syscreate(testSend1, 1024*4);
  syscreate(testRecv1, 1024*4);
  for (;;){
    sysyield();
  }
  return;
};
void testSend1(void){
  char buf[256];
  char printBuf[512];
  int result;
  senderPid = sysgetpid();
  while (recverPid == -1) {
    kprintf("TS: yieldy\b", recverPid);
    sysyield();
  }
  result = syssend(recverPid, buf, -1);
  if (result != -3) {
    sprintf(printBuf, "TS: ERR: neg buflen but: %d\n",result);
    sysputs(printBuf);
  }
  result = syssend(senderPid, buf, 128);
  if (result != -2) {
    sprintf(printBuf, "TS: ERR: neg calling self but: %d\n",result);
    sysputs(printBuf);
  }
  result = syssend(1234, buf, 128);
  if (result != -1) {
    sprintf(printBuf, "TS: ERR: nonexistent pid but: %d\n",result);
    sysputs(printBuf);
  }
  sprintf(printBuf, "TS: test1 pass\n", result);
  sysputs(printBuf);
  sysstop();
  return;
};

void testRecv1(void){
  char buf[256];
  char printBuf[512];
  int result;
  PID_t badPid = 44;
  recverPid = sysgetpid();
  while (senderPid == -1) {
    kprintf("TR: yieldy\n", recverPid);
    sysyield();
  }
  //sprintf(printBuf, "TR: recving for %d \n",senderPid);
  //sysputs(printBuf);
  result = sysrecv(&senderPid, buf, -1);
  if (result != -3) {
    sprintf(printBuf, "TR: ERR neg buflen but: %d \n", result);
    sysputs(printBuf);
  }
  result = sysrecv(&recverPid, buf, 128);
  if (result != -2) {
    sprintf(printBuf, "TR: ERR calling self but: %d \n", result);
    sysputs(printBuf);
  }
  result = sysrecv(&badPid, buf, 128);
  if (result != -1) {
    sprintf(printBuf, "TR: ERR nonexistent pid but result: %d \n", result);
    sysputs(printBuf);
  }

  sprintf(printBuf, "TR: test1 pass\n", result);
  sysputs(printBuf);
  sysstop();
  return;
}

void testIpc2(void) {
  //senderPid = -1;
  //recverPid = -1;
  initPids();
  syscreate(testSend2, 1024*4);
  syscreate(testRecv2, 1024*4);
  syscreate(testIpc2p3, 1024*4);
  for (;;){
    sysyield();
  }
  return;
};
void testSend2(void){
  char buf[256];
  char printBuf[512];
  pids[0] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("TS: yieldy\n");
    sysyield();
  }
  for (int i=0; i<3; i++) {
    sprintf(printBuf, "TS: w ");
    sysputs(printBuf);
  }
  syssend(pids[1], buf, 0);
  sprintf(buf, "some dance to forget");
  syssend(pids[1], buf, 21);
  /*
  if (result != -3) {
    sprintf(printBuf, "TS: ERR: neg buflen but: %d\n",result);
    sysputs(printBuf);
  }
  */
}
void testRecv2(void){
  char buf[256];
  char printBuf[512];
  int result;
  PID_t zeroPid = 0;
  PID_t sender;
  //recverPid = sysgetpid();
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("TR: yieldy\n");
    sysyield();
  }
  //sprintf(printBuf, "TR: recving for %d \n",senderPid);
  //sysputs(printBuf);
  sender = pids[0];
  result = sysrecv(&sender, buf, 200);
  for (int i=0; i<3; i++) {
    sprintf(printBuf, "TR: w ");
    sysputs(printBuf);
  }
  result = sysrecv(&zeroPid, buf, 50);
  
  sprintf(printBuf,"T[%d] R(%d) L:%d S:%s\n", pids[1], zeroPid, result, buf);
  sysputs(printBuf);
  /*
  for (int i=0; i<3; i++) { 
    sprintf(printBuf, "TR: w ");
    sysputs(printBuf);
  }
  */
  sender = pids[0];
  result = sysrecv(&sender, buf, 200);

  if (result < 0) {
    sprintf(printBuf, "TR: ERR: %d \n", result);
    sysputs(printBuf);
  }

  sprintf(printBuf,"T[%d] R(%d) L:%d S:%s\n", pids[1], sender, result, buf);
  sysputs(printBuf);

  result = syssend(pids[2], buf, result);

  sprintf(printBuf,"T[%d] S(%d) L:%d\n", pids[1], pids[2], result);
  sysputs(printBuf);
}
void testIpc2p3(void) {
  char buf[256];
  char printBuf[512];
  int result;
  PID_t sender;
  pids[2] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T%d: yieldy\n", pids[2]);
    sysyield();
  }
  sprintf(buf, "some dance to remember");
  result = syssend(pids[1], buf, 23);
  sprintf(printBuf,"T[%d] S(%d) L:%d\n", pids[2], pids[1], result);
  sysputs(printBuf);
  sender = pids[1];
  result = sysrecv(&sender, buf, 128);
  sprintf(printBuf,"T[%d] R(%d) L:%d S:%s\n", pids[2], sender, result, buf);
  sysputs(printBuf);

}
void testIpc3(void) {
  //senderPid = -1;
  //recverPid = -1;
  initPids();
  syscreate(testIpc3p1, 1024*4);
  syscreate(testIpc3p0, 1024*4);
  for (;;){
    sysyield();
  }
  return;
};
void testIpc3p0(void) {
  char buf[256];
  char printBuf[512];
  int result;
  pids[0] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T0[%d]: yieldy\n", pids[0]);
    sysyield();
  }
  sprintf(buf, "Jesus loves me this I know, for the Bible tells me so");
  result = syssend(pids[1], buf, 64);
  sprintf(printBuf, "T0[%d]>%d\n", pids[0], result);
  sysputs(printBuf);
}
void testIpc3p1(void) {
  char buf[256];
  char printBuf[512];
  int sender;
  int result;
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  sender = pids[0];
  result = sysrecv((PID_t*) &sender, buf, 16);
  sprintf(printBuf, "T1[%d], [%d]>%d\n", pids[1], sender, result);
  sysputs(printBuf);
  sysputs(buf);
}

void testIpc4(void) {
  initPids();
  syscreate(testIpc4p0, 1024*4);
  syscreate(testIpc4p1, 1024*4);
  syscreate(testIpc4p2, 1024*4);
  syscreate(testIpc4p3, 1024*4);
  syscreate(testIpc4p4, 1024*4);
  for(int i=0;i<30;i++){
    sysyield();
  }
  syskill(pids[0],31);
  for(int i=0;i<30;i++){
    sysyield();
  }
  //syskill(pids[1]);

  for (;;){
    sysyield();
  }
  return;
}

void testIpc4p0(void) {
  pids[0] = sysgetpid();
  //while (pids[0] == -1) {
  //  kprintf("T1[%d]: yieldy\n", pids[1]);
  //  sysyield();
  //}
  for(int i=0;i<10;i++){
    sysyield();
  }
  for(;;){
    sysyield();
  }
}

void testIpc4p1(void) {
  char buf[256];
  char printBuf[512];
  int result;
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  result=syssend(pids[0], buf, 64);
  sprintf(printBuf,"T[%d]: got %d",pids[1], result);
  sysputs(printBuf);

  sprintf(buf, "bird is the word");
  //result=syssend(pids[4], buf, 64);
  //sprintf(printBuf,"T[%d]: got %d",pids[1], result);
  //sysputs(printBuf);

  for(;;){
    sysyield();
  }
  }

void testIpc4p2(void) {
  char buf[256];
  char printBuf[512];
  // int sender;
  int result;
  pids[2] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
  }
  result=syssend(pids[0], buf, 128);
  sprintf(printBuf,"T[%d]: got %d",pids[2], result);
  sysputs(printBuf);
  //sender = pids[1];
  //result=sysrecv(&sender, buf, 120);
  //sprintf(printBuf,"T[%d]: got %d",pids[2], result);
  //sysputs(printBuf);
  for(;;){
    sysyield();
  }
}

void testIpc4p3(void) {
  char buf[256];
  char printBuf[512];
  int sender;
  int result;
  pids[3] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  sender = pids[0];
  result=sysrecv((PID_t *) &sender, buf, 256);
  sprintf(printBuf,"T[%d]: got %d",pids[3], result);
  sysputs(printBuf);
  sender = pids[1];
  //result=sysrecv(&sender, buf, 72);
  //sprintf(printBuf,"T[%d]: got %d",pids[3], result);
  //sysputs(printBuf);
  for(;;){
    sysyield();
  }
}

void testIpc4p4(void) {
  char buf[256];
  char printBuf[512];
  PID_t sender;
  int result;
  pids[4] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  sender = pids[0];
  result=sysrecv(&sender, buf, 256);
  sprintf(printBuf,"T[%d]: got %d %s\n",pids[4], result, buf);
  sysputs(printBuf);
  for(;;){
    sysyield();
  }

}
