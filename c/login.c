/* login.c : login interface
 */

#include<xeroskernel.h>
#include<xeroslib.h>
#include<program.h>


void mainProgram( void ) {
  //char  pBuf[64];
  int shellPid;
  int s;
  while (1) {
    s = startRoutine();
    if (s == 1) {
      shellPid = syscreate(&shell, 1024*5);
      syswait(shellPid);
    }
  }
}

int startRoutine( void ) {
  char pBuf[64];
  int fd;
  char ubuf[32]; // username buffer
  int buflen;
  int r;
  char pwbuf[32]; // password buffer
  int bkgd;
  // Print banner
  sprintf(pBuf, "Welcome to Xeros - an experimental OS\n");
  sysputs(pBuf);
  
  fd = sysopen(KBD1);
  
    // Username
    sprintf(pBuf, "Username: ");
    sysputs(pBuf);
    
    buflen = 32;
    r = sysread(fd, ubuf, buflen);
    if (r>0) {
      bkgd = cropInput(ubuf, &r);
      // turn echoiing off
      sysioctl(fd, KBD_IOCTL_OFF);
      sprintf(pBuf, "Password: ");
      sysputs(pBuf);

      r = sysread(fd, pwbuf, buflen);
      if (r>0) {
        bkgd |= cropInput(pwbuf, &r);
        if ((strncmp(ubuf,"cs415",5)==0) &&
              (strncmp(pwbuf,"EveryonegetsanA",15)==0) &&
              (bkgd == 0)) {
          sysputs("\n");
          sysclose(fd);
          return 1;
        }
      } else {
        if (r == 0) {
        }
      }
    } else {
      if (r == 0) {
      }
    }

  sysclose(fd);
  return 0;
}

// crops white space and overwrites buffer
// updates len according to amount cropped
// returns TRUE if & is present in the end, otherwise FALSE
int cropInput(char *buf, int *len) {
  int start; int end;
  int bkgd;
  start = 0; end = *len-1;

  //char pBuf[16];
  //sysputs(pBuf);
  while ((start < *len) && isWhiteSpace(&buf[start])) {  
    start++;
  }
  while ((end >= 0) && isWhiteSpace(&buf[end])) {
    end--;
  }
  bkgd = strncmp(&buf[end],"&",1) == 0;
  if (bkgd) {
    end--;
    while ((end >= 0) && isWhiteSpace(&buf[end])) {
      end--;
    }
  }
  *len = calculateLen(start, end);
  strncpy(buf, &buf[start], *len);
  return bkgd; 
}

int isWhiteSpace(char *c) {
  return ((strncmp(c," ",1) == 0) || (strncmp(c, "\t", 1) == 0) \
    || (strncmp(c, "\n", 1) == 0));
}

int calculateLen(int start, int end) {
  if (start > end) {
    return 0;
  }
  return end - start + 1;
}
