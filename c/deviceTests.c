#include <xeroskernel.h>
#include <xeroslib.h>
#include <tests.h>

void testDev0(void) {
  int fd;
  int r;
  char pBuf[128];
  
  //for (int i=0; i<6000000; i++) {}
  
  fd = sysopen(KBD0);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  for (int i=0; i<4000000; i++) {}
  sprintf(pBuf, "U: closing time!: %d\n",fd);
  sysputs(pBuf);
  if (fd >= 0) {
    r = sysclose(fd);
    sprintf(pBuf, "U: pool's closed!: %d\n", r);
    sysputs(pBuf);
  }

  fd = sysopen(KBD1);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  for (int i=0; i<4000000; i++) {}
  if (fd >= 0) {
    r = sysclose(fd);
    sprintf(pBuf, "U: pool's closed!: %d\n", r);
    sysputs(pBuf);
  }
  for(;;);
}

void testDev1(void) {
  // perform syswrite, expect -1
  int fd;
  int r;
  char pBuf[128];
  
  char buf[64];
  int buflen;
  //for (int i=0; i<6000000; i++) {}
  
  fd = sysopen(KBD1);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  
  for (int i=0; i<4000000; i++) {}
  if (fd >= 0) {
    buflen = 3;
    r = syswrite(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: syswrite err: %d\n",r);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: syswrite good: %d\n",r);
      sysputs(pBuf);
    }
  }
  for(;;);
}

void testDev2(void) {
  int fd;
  int r;
  char pBuf[128];
  
  char buf[64];
  int buflen;
  //for (int i=0; i<6000000; i++) {}
  
  fd = sysopen(KBD1);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  
  for (int i=0; i<4000000; i++) {}
  if (fd >= 0) {
    buflen = 3;
    sprintf(pBuf, "U: commence read!\n");
    sysputs(pBuf);
    r = sysread(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: sysreed: %d - %s\n",r, buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: sysreed whaaat: %d - %s\n",r, buf);
      sysputs(pBuf);
    
    }
  }

  for (int i=0; i<4000000; i++) {}
  if (fd >= 0) {
    buflen = 3;
    sprintf(pBuf, "U: commence read!\n");
    sysputs(pBuf);
    r = sysread(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: sysreed: %d - %s\n",r, buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: sysreed whaaat: %d - %s\n",r, buf);
      sysputs(pBuf);
    
    }
  }

  for(;;);

}

void testDev3(void) {
  int fd;
  int r;
  char pBuf[128];
  
  char buf[64];
  int buflen;
  //for (int i=0; i<6000000; i++) {}
  
  fd = sysopen(KBD1);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  
  for (int i=0; i<4000000; i++) {}
  
  sysioctl(fd, 55);
  sprintf(pBuf, "ioctl echo off\n");
  sysputs(pBuf);

  
  buflen = 10;
  r = sysread(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: sysreed: %d - %s\n",r, buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: sysreed whaaat: %d - %s\n",r, buf);
      sysputs(pBuf);
    
    }
  sysioctl(fd, 56);
  sprintf(pBuf, "ioctl echo on\n");
  sysputs(pBuf);
  
  
  sysioctl(fd, 53, 7);
  sprintf(pBuf, "ioctl change eof\n");
  sysputs(pBuf);

  buflen = 10;
  r = sysread(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: sysreed: %d - %s\n",r, buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: sysreed whaaat: %d - %s\n",r, buf);
      sysputs(pBuf);
    
    }

  for(;;);  
}

void testDev4(void) {
  char pBuf[32];
  int fd;

  fd = sysopen(2);
  sprintf(pBuf,"sysopen returned with value %d\n", fd);
  sysputs(pBuf);
  for(;;);
}

void testDev5(void) {
  char pBuf[32];
  char buf[8];
  int r;
  r = syswrite(-1, buf, 2);
  sprintf(pBuf,"syswrite returned with value %d\n", r);
  sysputs(pBuf);
}

void testDev6(void) {
  char pBuf[32];
  int fd;
  int r;

  fd = sysopen(KBD0);
  r = sysioctl(fd, 54);
  sprintf(pBuf,"sysioctl returned with value %d\n", r);
  sysputs(pBuf);
}

void testDev7(void) {
  int fd;
  int r;
  char pBuf[128];
  
  char buf[64];
  int buflen;
  //for (int i=0; i<6000000; i++) {}
  
  fd = sysopen(KBD1);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  
  for (int i=0; i<4000000; i++) {}
  if (fd >= 0) {
    buflen = 2;
    r = sysread(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: first sysread returns: %s\n", buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: syswrite good: %d\n",r);
      sysputs(pBuf);
    }

    r = sysread(fd, buf, buflen);
    if (r != -1) {
      sprintf(pBuf, "U: second sysread returns: %s\n", buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: syswrite good: %d\n",r);
      sysputs(pBuf);
    }

  }
  for(;;);

}

void testDev8(void) {
  int fd;
  int r;
  char pBuf[128];
  
  char buf[64];
  int buflen;
  //for (int i=0; i<6000000; i++) {}
  
  fd = sysopen(KBD1);
  sprintf(pBuf, "U: sysopen fd: %d\n",fd);
  sysputs(pBuf);
  
  for (int i=0; i<4000000; i++) {}
  if (fd >= 0) {
    buflen = 3;
    r = sysread(fd, buf, buflen);
    sysputs("\n");
    if (r != -1) {
      sprintf(pBuf, "U: sysread returns: %s\n", buf);
      sysputs(pBuf);
    } else {
      sprintf(pBuf, "U: syswrite good: %d\n",r);
      sysputs(pBuf);
    }
  }
  for(;;);
}
