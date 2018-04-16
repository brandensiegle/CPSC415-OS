/* di_calls.c : device independent calls
 */

#include <xeroskernel.h>
#include <i386.h>

devsw devTab[DTAB_SIZE]; // Device Table

/*
 * Function: di_open
 * --------------------
 * Opens device and returns file descriptor
 *
 * @param: PCB of process
 * @param: Major device number
 * @return: Index of fdt, or -1 if open failed
 *
 */
int di_open(pcb *process, int device_no) {
  // check dev_no
  // find free fdt
  // write in fdt
  // enable kbd irq
  int fd;
  int status;
  devsw *devptr;
  if (isValidDeviceNumber(device_no) == FALSE) {
    kprintf("K:di: bad devno\n");
    ready(process);
    return -1;
  }
  fd = findFreeFD(process);
  if (fd == EMPTY) {
    kprintf("K:di: no free fd\n");
    ready(process);
    return -1;
  }
  devptr = &devTab[device_no];
  if (fillFD(process, fd, devptr) == FAILED) {
    kprintf("K:di: fill fd error %d \n", fd);
    ready(process);
    return -1;
  }
  devptr = process->FD_tab[fd].dvBlock;
  if (devptr->dvopen == NULL) {
    kprintf("K:diopen: null ptr \n", process->pid);
    clearFD(process, fd);
    ready(process);
    return -1;
  }
  status = (devptr->dvopen)(process);
  //enable_irq(1,0);
  if (status < 0) {
    kprintf("K:diopen: dvopen failed %d\n", status);
    clearFD(process, fd);
    return -1;
  }
  return fd;
}
/*
 * Function: di_close
 * --------------------
 * Closes device 
 *
 * @param: PCB of process
 * @param: File descriptor
 * @return: 0 if close is successful, otherwise -1
 *
 */
int di_close(pcb *process, int fd) {
  int status;
  devsw *devptr;
  status = isValidFd(process, fd);
  if (status == FALSE) {
    kprintf("K:dicl: bad fd %d\n", fd);
    ready(process);
    return -1;
  }
  devptr = process->FD_tab[fd].dvBlock;
  if (devptr->dvclose == NULL) {
    kprintf("K:diclose: null ptr %d\n", process->pid);
    ready(process);
    return -1;
  }
  status = (devptr->dvclose)(process);
  if (status < 0) {
    return -1;
  }
  clearFD(process, fd);
  return 0;
}
/*
 * Function: di_write
 * --------------------
 * Writes to device 
 *
 * @param: PCB of process
 * @param: File descriptor
 * @param: Buffer for write
 * @paran: Number of bytes to write
 * @return: Number of bytes actually written, 
 *          -1 if error occured
 *
 */
int di_write(pcb *process, int fd, void* buffer, int buffer_len) {
  int status;
  devsw *devptr;
  if (!isValidBuffer(buffer, buffer_len)) {
    kprintf("K:diw: bad buffer %d\n", buffer_len);
    ready(process);
    return -1;
  }
  if (isValidFd(process, fd) == FALSE) {
    kprintf("K:diw: bad fd %d\n", fd);
    ready(process);
    return -1;
  }
  devptr = process->FD_tab[fd].dvBlock;
  if (devptr->dvwrite == NULL) {
    kprintf("K:diw: null ptr %d\n", process->pid);
    ready(process);
    return -1;
  }
  status = (devptr->dvwrite)(process, buffer, buffer_len);
  if (status < 0) {
    return -1;
  }
  return status;
}
/*
 * Function: di_read
 * --------------------
 * Reads from device
 *
 * @param: PCB of process
 * @param: File descriptor
 * @param: Buffer for read
 * @param: Number of bytes for read
 * @return: Number of bytes actully read
 *           0 if EOF
 *          -1 if error occured
 *
 */
int di_read(pcb *process, int fd, void* buffer, int buffer_len) {
  int status;
  devsw *devptr;
  if (!isValidBuffer(buffer, buffer_len)) {
    kprintf("K: dird: bad buffer %d\n", buffer_len);
    ready(process);
    return -1;
  }
  if (isValidFd(process, fd) == FALSE ) {
    kprintf("K: dird: bad fd %d \n", fd);
    ready(process);
    return -1;
  }
  devptr = process->FD_tab[fd].dvBlock;
  if (devptr->dvread == NULL) {
    kprintf("K: dird: null ptr %d\n", process->pid);
    ready(process);
    return -1;
  }
  status = (devptr->dvread)(process, buffer, buffer_len);
  if (status < 0) {
    return -1;
  }
  return status;
}
/*
 * Function: di_ioctl
 * --------------------
 * Sends ioctl call to device
 *
 * @param: PCB of process
 * @param: File descriptor
 * @param: Control command number
 * @param: Pointer to va_list of syscall
 * @return: 0 if successful, -1 otherwise
 *
 */
int di_ioctl(pcb *process, int fd, unsigned long command, void* va_listptr) {
  int status;
  devsw *devptr;
  if (isValidFd(process, fd) == FALSE) {
    kprintf("K: di ioctl: bad fd %d\n", fd);
    ready(process);
    return -1;
  }
  devptr = process->FD_tab[fd].dvBlock;
  if (devptr->dvioctl == NULL) {
    kprintf("K: di ioctl: null ptr %d\n", process->pid);
    ready(process);
    return -1;
  }
  status = (devptr->dvioctl)(process, command, va_listptr);
  if (status < 0) {
    return -1;
  }
  return status;
}

/*
 * Function: init_devTab
 * --------------------
 * Initialize kernel device table
 * 
 * @param: PCB of process
 * @param: device table
 * @return: OK if successful, FAILED otherwise
 *
 */
int devTabInit( void ) {
  int i;
  for (i=0; i<DTAB_SIZE; i++) {
    devSWInit(&devTab[i]);
  }
  kbd0_init(&devTab[KBD0]);
  kbd1_init(&devTab[KBD1]);
  kprintf("init devTab\n");
  return OK;
}

void devSWInit(devsw *devptr) {
  // initialize fd table entry
  devptr->dvnum = EMPTY;
  devptr->dvopen = NULL;
  devptr->dvclose = NULL;
  devptr->dvwrite = NULL;
  devptr->dvread = NULL;
  devptr->dvioctl=NULL;
}
  
int isValidFd(pcb *process, int fd) {
  // validate file descriptor
  // return TRUE/FALSE
  int result = TRUE;
  if (fd >= FDT_SIZE || fd<0) {
    result = FALSE;
  } else {
    if (process->FD_tab[fd].dvBlock == (devsw *) NULL) {
      result = FALSE;
    }
  }
  return result;
}

int isValidDeviceNumber(int devno) {
  int result = TRUE;
  if (devno >= DTAB_SIZE) {
    return FALSE;
  }
  if (devTab[devno].dvnum == EMPTY) {
    return FALSE;
  }
  return result;
}


