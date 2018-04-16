#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <tests.h>

static PID_t pids[5];


void testIpc5(void) {
  //senderPid = -1;
  //recverPid = -1;
  initPids();
  syscreate(testIpc5p1, 1024*4);
  syscreate(testIpc5p0, 1024*4);
  for (;;){
    sysyield();
  }
  return;
};
void testIpc5p0(void) {
  char buf[256];
  char printBuf[512];
  int result;
  pids[0] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T0[%d]: yieldy\n", pids[0]);
    sysyield();
  }
  sprintf(buf, "Jesus loves me this I know, for the Bible tells me so");
  result = syssend(pids[1], buf, 0);
  sprintf(printBuf, "P:A[%d] L:%d\n", pids[0], result);
  sysputs(printBuf);
  for(;;){ sysyield();}
}
void testIpc5p1(void) {
  char buf[256];
  char printBuf[512];
  PID_t sender;
  int result;
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  sender = 0;
  result = sysrecv(&sender, buf, 16);
  sprintf(printBuf, "P:B[%d], S:[%d] L:%d buf:%s\n", pids[1], sender, result, buf);
  sysputs(printBuf);
  for(;;){ sysyield();}
}

void testIpc6(void) {
  initPids();
  syscreate(testIpc6p0, 1024*4);
  syscreate(testIpc6p1, 1024*4);
  syscreate(testIpc6p2, 1024*4);
  for (;;){
    sysyield();
  }
  return;
};
void testIpc6p0(void) {
  char buf[256];
  char printBuf[512];
  int result;
  PID_t sender;
  pids[0] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T0[%d]: yieldy\n", pids[0]);
    sysyield();
  }
  for(int i=0; i<20; i++) {
    sysyield();
  }
  sender = 0;
  result = sysrecv(&sender, buf, 64);
  sprintf(printBuf, "P:[%d] R:[%d] L:%d S:%s\n", pids[0], sender, result, buf);
  sysputs(printBuf);
  sender = 0;
  result = sysrecv(&sender, buf, 64);
  sprintf(printBuf, "P:[%d] R:[%d] L:%d S:%s\n", pids[0], sender, result, buf);
  sysputs(printBuf);

  for(;;){ sysyield();}
}
void testIpc6p1(void) {
  char buf[256];
  char printBuf[512];
  int result;
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T0[%d]: yieldy\n", pids[0]);
    sysyield();
  }
  sprintf(buf, "We have an anchor that keeps the soul");
  result = syssend(pids[0], buf, 128);
  sprintf(printBuf, "P:[%d] R:[%d] L:%d\n", pids[1], pids[0], result);
  sysputs(printBuf);
  for(;;){ sysyield();}
}
void testIpc6p2(void) {
  char buf[256];
  char printBuf[512];
  int result;
  pids[2] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T0[%d]: yieldy\n", pids[0]);
    sysyield();
  }
  sprintf(buf, "steadfast and still while the billows roll");
  result = syssend(pids[0], buf, 256);
  sprintf(printBuf, "P:[%d] R:[%d] L:%d\n", pids[2], pids[0], result);
  sysputs(printBuf);
  for(;;){ sysyield();}
}


void testIpc7(void) {
  initPids();
  syscreate(testIpc7p0, 1024*4);
  syscreate(testIpc7p1, 1024*4);
  syscreate(testIpc7p2, 1024*4);
  syscreate(testIpc7p3, 1024*4);
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

void testIpc7p0(void) {
  //pids[0] = sysgetpid();
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

void testIpc7p1(void) {
  char buf[256];
  char printBuf[512];
  //PID_t sender;
  int result;
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  result=syssend(pids[0], buf, 64);
  sprintf(printBuf,"T[%d]: got %d",pids[1], result);
  sysputs(printBuf);

  sprintf(buf, "streams of mercy never ceasing, calls forth song of loudest praise");
  result=syssend(pids[3], buf, 100);
  sprintf(printBuf,"T[%d]: got %d",pids[1], result);
  sysputs(printBuf);

  for(;;){
    sysyield();
  }
  }

void testIpc7p2(void) {
  char buf[256];
  char printBuf[512];
  PID_t sender;
  int result;
  pids[2] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
  }
  sender = pids[0];
  result=sysrecv(&sender, buf, 128);
  sprintf(printBuf,"T[%d]: got %d",pids[2], result);
  sysputs(printBuf);
  sender = pids[1];
  for(;;){
    sysyield();
  }
}


void testIpc7p3(void) {
  char buf[256];
  char printBuf[512];
  PID_t sender;
  int result;
  pids[3] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  sender = pids[1];
  result=sysrecv(&sender, buf, 256);
  sprintf(printBuf,"T[%d]: got %d %s\n",pids[4], result, buf);
  sysputs(printBuf);
  for(;;){
    sysyield();
  }
}

void testIpc8(void) {
  initPids();
  syscreate(testIpc8p0, 1024*4);
  syscreate(testIpc8p1, 1024*4);

  for (;;){
    sysyield();
  }
  return;
}

void testIpc8p0(void) {
  char buf[256];
  char printBuf[512];
  //int sender;
  int result;
  pids[0] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }

  sprintf(printBuf,"S[%d] R[%d] L:%d",pids[0], pids[1], -1);
  sysputs(printBuf);
  result=syssend(pids[1], buf, -1);
  sprintf(printBuf,"T[%d]: got %d",pids[1], result);
  sysputs(printBuf);
  for(;;){
    sysyield();
  }
}

void testIpc8p1(void) {
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  
  for(;;){
    sysyield();
  }
}


void testIpc9(void) {
  initPids();
  syscreate(testIpc9p0, 1024*4);

  for (;;){
    sysyield();
  }
  return;
}

void testIpc9p0(void) {
  char buf[256];
  char printBuf[512];
  PID_t sender;
  int result;
  pids[0] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }

  sprintf(printBuf,"R[%d] S[%d] L:%d",pids[0], pids[0], 32);
  sysputs(printBuf);
  sender = pids[0];
  result=sysrecv(&sender, buf, 32);
  sprintf(printBuf,"T[%d]: got %d",pids[0], result);
  sysputs(printBuf);
  for(;;){
    sysyield();
  }
}


void testIpc10(void) {
  initPids();
  syscreate(testIpc10p0, 1024*4);
  syscreate(testIpc10p1, 1024*4);

  for (;;){
    sysyield();
  }
  return;
}

void testIpc10p0(void) {
  //char buf[256];
  char printBuf[512];
  int sender;
  int result;
  pids[0] = sysgetpid();
  while (pids[1] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }

  sprintf(printBuf,"R[%d] S[%d] L:%d",pids[0], pids[1], 32);
  sysputs(printBuf);
  sender=pids[1];
  result=syssend(sender, NULL, 32);
  sprintf(printBuf,"T[%d]: got %d",pids[0], result);
  sysputs(printBuf);
  for(;;){
    sysyield();
  }
}

void testIpc10p1(void) {
  pids[1] = sysgetpid();
  while (pids[0] == -1) {
    kprintf("T1[%d]: yieldy\n", pids[1]);
    sysyield();
  }
  
  for(;;){
    sysyield();
  }
}


