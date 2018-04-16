/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */
#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   	0       /* Boolean constants             */
#define	TRUE    	1
#define	EMPTY   	(-1)    /* an illegal gpq                */
#define TOMBSTONE	-2
#define	NULL    	0       /* Null pointer for linked lists */
#define	NULLCH 		'\0'    /* The null character            */


/* Universal return constants */
#define	OK            1         /* system call ok               */
#define FAILED	      0		/*Other failure			*/
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

/* Dispatcher request constants */
#define CREATE        41         /* create process	*/
#define YIELD         42         /* yield process 	*/
#define STOP          43         /* stop process 	*/
#define KILL          44         /* kill process 	*/
#define PUTS          45         /* put to screen 	*/
#define GETPID        46         /* get pid of process	*/
#define SEND          47         /* send buffer to another process */
#define RECV          48         /* receive buffer to another process */
#define SLEEP	      49	 /* Night night Zzzzz.. */
#define GETCPUTIMES      50      /* getcputimes */
#define WAIT	      51	 /* wait for process to end */
#define SIGRETURN     52	 /* return from handling signal */
#define SIGHANDLE     53	 /* install a signal handler */
#define DV_OPEN       54         /* open device */
#define DV_CLOSE      55         /* close device */
#define DV_WRITE      56         /* write to device */
#define DV_READ       57         /* read from device */
#define DV_IOCTL      58         /* device ioctl call */

#define TIMER_INT     32	
#define KBD_INT       33         /* keyboard interrupt */

/* PCB Constant(s) */
#define PCBTABLE_SIZE 32          /* 32 entries */
#define PAGE_SIZE     16       /* size of page is 16 bytes */
#define FDT_SIZE      4        /* size of file descriptor table */
/* Process States */
#define READY	      10
#define RUNNING	      11
#define BLOCKED	      12
#define SLEEPING      13
#define WAITING	      14


#define STOPPED       -1      /* since stopped entries are memset'd to -1 */

/* Time Slice Length */
#define TIME_SLICE    10	/* Number of milliseconds per time_slice */

/* Device Table */
#define DTAB_SIZE     2         /* Size of Device Table */
#define KBD0          0         /* Non-echoing Keyboard */
#define KBD1          1         /* Echoing Keyboard */

/* KBD Constants */
#define KBD_DP        0x60     /* Keyboard Data Port */
#define KBD_SP        0x64     /* Keyboard Status Port */
#define KBD_DATA_MASK 0x01     /* Mask to see if data is available */
#define KBD_IOCTL_EOF 53       /* Keyboard IOCTL cmd to change EOF char*/
#define KBD_IOCTL_OFF 55       /* Keyboard IOCTL cmd to turn echo off */
#define KBD_IOCTL_ON  56       /* Keyboard IOCTL cmd to turn echo on */
/* Functions defined by startup code */
void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);
void           set_evec(unsigned int xnum, unsigned long handler);

/* Defined types and data structures */
/** Memory Header **/
typedef struct struct_mh memHeader;
struct struct_mh {
  unsigned long size;       // size of memory block
  struct memHeader *prev;   // ptr to previous node
  struct memHeader *next;   // ptr to next node
  char *sanityCheck;        // check for integrity of memHeader
  // address to the end of memHeader (start of mem block)
  unsigned char dataStart[0];
};

/** Process CPU Status **/
typedef struct {
  unsigned long   edi;  
  unsigned long   esi;
  unsigned long   ebp;
  unsigned long   esp;
  unsigned long   ebx;
  unsigned long   edx;
  unsigned long   ecx;
  unsigned long   eax;
  unsigned long   iret_eip;
  unsigned long   iret_cs;
  unsigned long   eflags;
  unsigned long   extra; // round to 16B
} cpu; 

/** List of Memory Headers for a Process **/
typedef struct  struct_ma  memAlloc;
struct struct_ma {
  memHeader	*memLoc;	/* Allocated memory header */
  memAlloc	*next;		/* Next memory block allocated */
};

/** Resources Used by a Process **/
typedef struct {
  memAlloc	memoryAllocs;	/* List of memory allocations */
} procResources;

/** Signal Handler Table **/

/** Process Control Block **/
typedef struct struct_pcb pcb;

/** Device Table & FDT **/
typedef struct struct_dsw devsw;
struct struct_dsw {
  int dvnum;          /* Device major number */
  //char *dvname;       /* Device name */
  int (*dvopen)(pcb *p);   /* Func_ptr to device open */
  int (*dvclose)(pcb *p);  /* Func_ptr to device close */
  int (*dvwrite)(pcb *p, void* buffer, int buffer_len);
                                  /* Func_ptr to device write */
  int (*dvread)(pcb *p, void* buffer, int buffer_len);
                                  /* Func_ptr to device read */
  int (*dvioctl)(pcb *p, unsigned long command, void* va_listptr);
                                  /* Func_ptr to device ioctl */
};

typedef struct {
  devsw *dvBlock;   /* Device block */
} FD_entry;

struct struct_pcb {
  int   pid;			/* PID of process */
  int   state;			/* Current state of process */
  int   parent_pid;		/* Parent PID */
  cpu   cpu_state;		/* CPU State (registers and flags) */
  unsigned long	intRetVal;	/* Return value of system call */
  procResources resources;	/* Any resources held by process */
  int   sleepTime;		/* Remaining time to sleep */
  long  arguments;		/* Pointer to arguments passed during a system call */
  void *ipcArgs;   		/* Pointer to va_list of sender or receiver 
                         	containing void* buffer ptr and int buffer len*/
  int target_of_recv; 		/* The pid of process the current pcb is receiving to.
                          	A value of 0 signifies receive all
                          	Setting to it's own pid signifies not receiving */
  FD_entry FD_tab[FDT_SIZE]; /* File descriptor table */

		/* Previous process in the queue */
  int waitingFor;		/* The PID of the process waiting to end */
  pcb *waitingNext;	/* Pointer to next process waiting on this process */
  pcb *sleepNext;	/* Pointer to next item on the sleepList */
  pcb *sleepPrev;	/* Pointer to previous item on the sleepList */
  pcb *sender_queue; 	/* first blocked process sending to current process  */
  pcb *nextPCBEntry;	/* A pointer to the process with the next highest PID */
  pcb *next;		/* Next process in the queue */
  pcb *prev;		/* Previous process in the queue */

  unsigned long runningTime;

  unsigned long sigMaskInstalled;
  unsigned long sigMaskOnStack;
  unsigned long sigMaskQueued;
  
  void *installedHandlers[32];
};

/** ProcessStatuses Struct for getcputimes */
typedef struct struct_ps processStatuses;
struct struct_ps {
  int  entries;            // Last entry used in the table
  int  pid[PCBTABLE_SIZE];      // The process ID
  int  status[PCBTABLE_SIZE];   // The process status
  long  cpuTime[PCBTABLE_SIZE]; // CPU time used in milliseconds
};




/** PID_t type for Identifying a Process **/
typedef unsigned int PID_t;

/*
 *
 * Additional Functions
 *
 */
/** Signal Functions **/ 				
void	sigtramp(void (*handler)(void *), void *cntx);  
int	signal(int PID, int sigNumber);	
int	handleRegister(pcb *process, int signalNum, void *newHandler, void **oldHandler);		

/** Memory Functions **/
void	   	kmeminit(void);
void	  	*kmalloc(size_t size);
int	    	kfree(void *ptr);
memHeader	*backTrackMemHeader(void *ptr);
int       	checkSanityCheck(char *sanityCheck, void *ptr);
void      	handleKFreeError(void);

/** Memory Header Functions **/
int       initializeMemHeaderList(void);
int       insertMemHeaderLink(memHeader* memPtr);
int       linkMemHeaderInFront(memHeader* newHeader, memHeader* memPtr);
int       linkMemHeaderBehind(memHeader* memPtr, memHeader* newHeader);
memHeader *getMemHeader(size_t size);
int       checkAndMergeMemHeaders(memHeader* linkedPtr, memHeader* newPtr);
void      *roundUpMemoryAddress(void *addr);
int       isValidMemoryAddress(void *addr);
int       printMemHeaders(void);
void      printMemHeaderHelper(memHeader* memPtr);

/** Dispatcher Functions **/
void initializeQueues(void);
void	  dispatch(void);
pcb       *next(void);
void      ready(pcb* process); 
void      cleanup(pcb* process);
void      printReadyQueue(void);
pcb	  *getRunningProc(void);

/** PCB Functions **/
void	  initializeQueues(void);
void      pcbTableInit(void);
pcb       *findFreePcb(void);
void	  removeProcess(pcb* process);
void      printPcbTable(void);
void      printPcb(pcb* pcbEntry);
pcb	  *getPcb(PID_t);
int       getCPUtimes(pcb *p, processStatuses *ps);
int       initializeFDT(pcb *process);
int       findFreeFD(pcb *process);
int       fillFD(pcb *process, int fd, devsw *devptr);
void      clearFD(pcb *process, int fd);
void	  addCPUTimes(int milliseconds);

/** Context Switcher Functions **/
void 	  contextinit(void);
int       contextswitch(pcb* process);
void      _ISREntryPoint(void);
void	  _TimerEntryPoint(void);
void    _KBDEntryPoint(void);
void	  _CommonJump(void);	

/** Process Create Functions **/
int       create(void (*func)(void), int stack);

/** User Functions **/
void	  root(void);



/** System Call Functions **/
int syscall(int call, ...);
extern unsigned int syscreate( void (*func)(void), int stack );
extern void sysyield( void );
extern void sysstop( void );
extern void sysputs( char *str );
extern PID_t sysgetpid( void );
//extern int syskill( PID_t pid ); ##  REPURPOSED FOR SIGNALLING ##
extern int syssend( PID_t dest_pid, void *buffer, int buffer_len );
extern int sysrecv( PID_t *from_pid, void *buffer, int buffer_len ); 
extern unsigned int syssleep( unsigned int milliseconds );
extern int syswait(int PID);  
extern int syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *));	
extern void syssigreturn(void *old_sp);		
extern int syskill(int PID, int signalNumber);	
extern int sysgetcputimes(processStatuses *ps);
extern int sysopen(int device_no);
extern int sysclose(int fd);
extern int syswrite(int fd, void *buffer, int buffer_len);
extern int sysread(int fd, void *buffer, int buffer_len);
extern int sysioctl(int fd, unsigned long command, ...);


extern void syspcbprint( void );


/** Sleeping Functions **/
extern int sleep(unsigned int milliseconds);
extern void tick( void );
extern void sleepInit(void);
extern int removeFromSleepList(pcb* process);
void wake(pcb* process);

/** Messaging System Functions **/
extern int send( PID_t dest_pid, void *buffer, int buffer_len, pcb *senderPCB);
extern int recv( PID_t *from_pid, void *buffer, int buffer_len, pcb *receiverPCB);
int   isCallingToSelf(PID_t *target_pid, pcb *caller_pcb);
int   isValidBuffer(void *buffer, int buffer_len);
int   runSend(pcb *dest_PCB, pcb *senderPCB);
int   blockSend(pcb *dest_PCB, pcb *senderPCB);
int   blockRecv(PID_t senderPid, pcb *recverPCB);
int   transferBuffer(pcb *dest_PCB, pcb *senderPCB);
//int   minBufLen(int recvBufLen, int sendBufLen);

/** Messaging PCB Functions **/
int   isProcReceiving(pcb *recverPCB, PID_t senderPid);
int   addBlockedSender(pcb *recverPCB, pcb *senderPCB);
int   rmBlockedSender(pcb *recverPCB, pcb *senderPCB);
int   isProcSending(PID_t senderPid, pcb *recverPCB);
pcb   *getFirstSender(pcb *recverPCB);
int   addBlockedRecver(PID_t senderPid, pcb *recverPCB);
int   rmBlockedRecver(pcb *recverPCB);
void  printSenderQueue(pcb *recverPCB);
int   cleanUpSenders(pcb *cleanUpPcb);
pcb*  cleanUpSendersHelper(pcb *cleanUpPcb, pcb *senderPCB);
int   cleanUpRecvers(pcb *cleanUpPcb);
int   getAllRecvers(PID_t cleanUpPid, PID_t *pidArray);
int   cleanUpRecversHelper(pcb *recverPCB);

/** Device Independent Calls **/
int di_open(pcb *process, int device_no);
int di_close(pcb *process, int fd);
int di_write(pcb *process, int fd, void* buffer, int buffer_len);
int di_read(pcb *process, int fd, void* buffer, int buffer_len);
int di_ioctl(pcb *process, int fd, unsigned long command, void* va_listptr);
int devTabInit(void);
void devSWInit(devsw *devptr);
int isValidFd(pcb *process, int fd);
int isValidDeviceNumber(int devno);

// Keyboard reader context
typedef struct {
  pcb *p;         // pcb of process calling sysread
  unsigned char *buf;      // process read buffer
  int buflen;     // requested read length
  int i;           // index of read buffer marking how much data is written
} kbd_ct;

/** General Keyboard Primitives */
int kbdG_open(pcb *p, int echo);
int kbdG_close(pcb *p);
int kbdG_write(pcb *p, void* buffer, int buffer_len);
int kbdG_read(pcb *p, void* buffer, int buffer_len);
int kbdG_ioctl(pcb *p, unsigned long command, void* va_listptr);
int initializeKbdState(int echo);
int kbdBufferTransfer(kbd_ct *kbd_pPtr);
int kbdWriteBuffer(unsigned char *buf, int *ind, int size, unsigned char data);
void kbdPrintDevBuf(void);
void kbdPrintProcBuf(void);
int kbdUnblockReadProc(kbd_ct *kbd_pPtr);
int kbdActivateEOF(kbd_ct *kbd_pPtr);

/** Keyboard 0 (non-echoing) primitives */
int kbd0_open(pcb *p);
//int kbd0_close(pcb *p);
//int kbd0_write(pcb *p, void* buffer, int buffer_len);
int kbd0_read(pcb *p, void* buffer, int buffer_len);
int kbd0_ioctl(pcb *p, unsigned long command, void* va_listptr);
int kbd0_init(devsw *devptr);

/** Keyboard 1 (echoing) primitives */
int kbd1_open(pcb *p);
//int kbd1_close(pcb *p);
//int kbd1_write(pcb *p, void* buffer, int buffer_len);
int kbd1_read(pcb *p, void* buffer, int buffer_len);
int kbd1_ioctl(pcb *p, unsigned long command, void* va_listptr);
int kbd1_init(devsw *devptr);
unsigned int handleKbdISR(void);


/** Improved Producer/Cunsumer Functions **/
extern void producer( void );
extern void consumer( void );

/** Idle Process **/
extern void idleproc( void );

/*
 *
 *Testing Functions
 *
 */

//  Memory testing functions
//  Call at your own risk!
int       countMemHeaders(void);
memHeader *getRootHeader(void);

/* Anything you add must be between the #define and this comment */
#endif
