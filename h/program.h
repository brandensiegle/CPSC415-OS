#ifndef PROGRAM_H
#define PROGRAM_H

/** Shell **/


void  mainProgram(void);
int  startRoutine(void);
void  password(void);
void  shell(void);
void  handleShellCmd(int pid, int fd, char *buf, int len, int bkgd);

int   cropInput(char *buf, int *len);
int   isWhiteSpace(char *c);
int   calculateLen(int start, int end);
void  parseInput(char *buf, int len, \
  char *cmdBuf, char *pmBuf, int *cmdLen, int *pmLen);
/** ps **/
void  prog_ps(void);    ///TODO

/** k **/
// return -1 if process does not exist
int   prog_k(int PID);	///TODO

/** a **/
// bkgd is either TRUE of FALSE
//  corresponds to whether there is an '&'
void alarmHandler(void);
void  prog_a(void);	///TODO

/** t **/
void  prog_t(void);	///TODO



#endif
