/* kbd.c - keyboard primitives */

#include<xeroskernel.h>
#include<xeroslib.h>
#include<i386.h>
#include<kbd.h>
#include<stdarg.h>

static int kbd_state;             
static unsigned char kbd_buf[KBD_BUF_SIZE];   // device buffer
static int kbd_buf_ind; // index of next free spot on dev buffer
static kbd_ct kbd_p;    // context of process calling sysread
static char eof_val;    // EOF indication

/*
 * Function: kbdG_open
 * --------------------
 * Initialize buffers and keyboard state
 *
 * @param: PCB of process
 * @param: boolean value for echoiing
 * @return: 0 if successful, -1 if fails
 *
 */

int kbdG_open(pcb *p, int echo) {
  // put process on ready queue
  ready(p);
  if (kbd_state != EMPTY) {
    kprintf("KBD: err: kbd already opened\n");
    return -1;
  }
  // init buffers
  kbd_buf_ind = 0;
  // set echo flags
  kbd_state = initializeKbdState(echo);
  // init kbd context
  kbd_p.p = NULL;
  // init eof value
  eof_val = C_D;
  // enable KBD IRQ
  enable_irq(1,0);
  //kprintf("KBD: open echo: %d\n", kbd_state & ECHO);
  return 0;
}
/*
 * Function: kbdG_close
 * --------------------
 * Close keyboard, disable KBD interrupts
 *
 * @param: PCB of process
 * @return: 0 if successful, -1 if fails
 *
 */

int kbdG_close(pcb *p) {
  //kprintf("KBD: close\n");
  ready(p);
  kbd_state = EMPTY;
  enable_irq(1,1);
  return 0;
}
/*
 * Function: kbdG_write
 * --------------------
 * Not implemented, returns -1
 *
 */

int kbdG_write(pcb *p, void* buffer, int buffer_len) {
  //kprintf("KBD: write\n");
  ready(p);
  return -1;
}

/*
 * Function: kbdG_read
 * --------------------
 * Checks keyboard buffer, block process
 * If EOF is triggered, return EOF
 *
 * @param: PCB of process
 * @param: Application buffer
 * @param: Number of bytes to write
 * @return: Number of bytes read if successful, 
 *          0 if EOF
 *          -1 if fails
 *
 */

int kbdG_read(pcb *p, void* buffer, int buffer_len) {
  int r;
  kbd_p.p = p;
  kbd_p.buf = (unsigned char *) buffer;
  kbd_p.buflen = buffer_len;
  kbd_p.i = 0;
  if (kbd_state & DISABLE) {
    kbdActivateEOF(&kbd_p);
    return 0;
  }
  r = kbdBufferTransfer(&kbd_p);
  if (kbd_p.i == kbd_p.buflen) {
    return kbdUnblockReadProc(&kbd_p);
  }
  return r;
}
/*
 * Function: kbdG_ioctl
 * --------------------
 * Runs IOCTL, 53 changes EOF, 55 disables echo, 56 enables echo
 *
 * @param: PCB of process
 * @param: Command Value
 * @param: va_list pointer
 * @return: 0 if succesful
 *          -1 if fails
 *
 */
int kbdG_ioctl(pcb *p, unsigned long command, void* va_listptr) {
  int r =0 ;
  switch( command ) {
    case (KBD_IOCTL_EOF):
    ;
      va_list *vArgp = (va_list *) va_listptr;
      eof_val =  va_arg(*vArgp, int);
    break;
    case (KBD_IOCTL_OFF):
      kbd_state &= ~ECHO;
    break;
    case (KBD_IOCTL_ON):
      kbd_state |= ECHO;
    break;
    default:
      r = -1;
    break;
  }
  ready(p);
  return r;
}

int initializeKbdState(int echo) {
  int state = 0;
  if (echo == TRUE) {
    state |= ECHO;
  }
  return state;
}

/* Keyboard 0 (non-echoing) primitives */
int kbd0_open(pcb *p) {
  return kbdG_open(p, FALSE);
}


int kbd0_init(devsw* devptr) {
  devptr->dvnum = KBD0;
  devptr->dvopen = &kbd0_open;
  devptr->dvclose = &kbdG_close;
  devptr->dvwrite = &kbdG_write;
  devptr->dvread = &kbdG_read;
  devptr->dvioctl = &kbdG_ioctl;
  kbd_state = EMPTY;
  return OK;
}

/** Keyboard 1 (echoing) primitives */
int kbd1_open(pcb *p) {
  return kbdG_open(p, TRUE);
}

int kbd1_init(devsw *devptr) {
  devptr->dvnum = KBD1;
  devptr->dvopen = &kbd1_open;
  devptr->dvclose = &kbdG_close;
  devptr->dvwrite = &kbdG_write;
  devptr->dvread = &kbdG_read;
  devptr->dvioctl = &kbdG_ioctl;
  kbd_state = EMPTY;
  return OK;
}


unsigned int handleKbdISR(void) {
  unsigned char st;
  unsigned char data;
  unsigned char d;
  st = inb(KBD_SP);
  int r;
  //kprintf("K: hKbdISR: 0x%x %d, %d\n", st,st,isData);
  if (st & KBD_DATA_MASK) {
    data = inb(KBD_DP);
    d = (unsigned char) kbtoa(data);
    //kprintf("K: hKbdISR: 0x%x %c\n", data, d);
    if (kbd_state & ECHO) {
      if (d != eof_val) {
        kprintf("%c", d);
      }
    }
    if (kbd_p.p == NULL) {
      kbdWriteBuffer(kbd_buf, &kbd_buf_ind, KBD_BUF_SIZE, d);   
    } else {
      r = kbdWriteBuffer(kbd_p.buf, &kbd_p.i, kbd_p.buflen, d);
      //kbdPrintProcBuf();
      if (kbd_p.i == kbd_p.buflen) {
        kbdUnblockReadProc(&kbd_p);
      }
      if (r == ENTER_V) {
        kbdUnblockReadProc(&kbd_p);
      }
      if (r == C_D_V) {
        kprintf("\n");
        kbdActivateEOF(&kbd_p);
      }
    }
  }
  return d;
}

int kbdBufferTransfer(kbd_ct *kbd_pPtr) {
  int charsToTransfer;
  int charsToShift;

  charsToTransfer = kbd_pPtr->buflen - kbd_pPtr->i;
  charsToTransfer = (charsToTransfer > (kbd_buf_ind))? \
    kbd_buf_ind : charsToTransfer;
  for (int j=0; j<charsToTransfer; j++) {

    kbd_pPtr->buf[kbd_pPtr->i] = kbd_buf[j];
    kbd_pPtr->i++;
    if (kbd_buf[j] == ENTER){
      charsToTransfer = j+1;
      kbdUnblockReadProc(kbd_pPtr);
    } 
    if (kbd_buf[j] == eof_val) {
      kbdActivateEOF(kbd_pPtr);
      return 0;
    }
  }
  if (charsToTransfer <= kbd_buf_ind) {
    // 'shift' kbd_buf
    charsToShift = kbd_buf_ind - charsToTransfer;
    for (int k=0; k<charsToShift; k++) {
      kbd_buf[k] = kbd_buf[k + charsToTransfer];
    }
    kbd_buf_ind = charsToShift;
  }
  //  update kbd_ct, kbd_buf_ind 
  //kprintf("KBD: done transfer \n");
  return kbd_pPtr->i;
}
int kbdUnblockReadProc(kbd_ct *kbd_pPtr) {
  kbd_pPtr->p->cpu_state.eax = kbd_pPtr->i;
  //kprintf("KBD: unblok %d %d\n", kbd_pPtr->p->cpu_state.eax, kbd_pPtr->i);
  ready(kbd_pPtr->p);
  return kbd_pPtr->i;
}

int kbdActivateEOF(kbd_ct *kbd_pPtr) {
  kbd_pPtr->p->cpu_state.eax = 0;
  ready(kbd_pPtr->p);
  kbd_state |= DISABLE;
  enable_irq(1,1);
  return 0;
}

int kbdWriteBuffer(unsigned char *buf, int *ind, int size, unsigned char data) {
  // handle data (enter, backspace)
  // kprintf("KBD: write buf %c[%d]\n", data, data);
  int r = 0;
  switch (data) {
    case (BKSPC):
      if (*ind > 0) {
        *ind = *ind-1;
      }
      return 0;
    break;
    case (ENTER):
      r = ENTER_V;
    break;
  }
  if ((data != NOCHAR) && (data != 0)) {
    //kprintf("KBD: chun \n");
    if (*ind < size) {
      //kprintf("KBD: write done \n");
      buf[*ind] = data;
      //sprintf(&buf[*ind], "%c", data);
      *ind = *ind + 1;
    }
  }
  if (data == eof_val) {
    return C_D_V;
  }
    
  return r;
}

void kbdPrintDevBuf( void ) {
  kprintf("K:devBuf: %d %s", kbd_buf_ind, kbd_buf);
  //for (int i=0; i<kbd_buf_ind; i++) {
  //  kprintf("%c", kbd_buf[i]);
  //}
  kprintf("\n");
}

void kbdPrintProcBuf( void ) {
  kprintf("K:procBuf: %d %d %s", kbd_p.i, kbd_p.buflen, kbd_p.buf);
  //for (int j=0; j<kbd_p.i; j++) {
  //  kprintf("%c", kbd_p.buf[j]);
  //}
  kprintf("\n");
}
