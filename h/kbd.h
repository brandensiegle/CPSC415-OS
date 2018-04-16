#ifndef KBD_H
#define KBD_H

/** KBD definitions **/
#define C_D           4               /* ASCII corresponding to Control-d */
#define KBD_BUF_SIZE  4                /* Size of keyboard read buffer */
#define BKSPC         0x08             /* Backspace character */
#define ENTER         0x0a             /* Linefeed character  */

/* KBD state flags */
#define ECHO          0x01             /* echo kbd input */
#define DISABLE       0x02             /* do not send to app, triggered by C-d*/

/* KBD Read Definitions */
#define ENTER_V       -1               /* notify kernel enter is pressed */
#define C_D_V         -2               /* notify kernel control-d is pressed */

/** scancodesToAscii definitions **/

#define KEY_UP   0x80            /* If this bit is on then it is a key   */
                                 /* up event instead of a key down event */

/* Control code */
#define LSHIFT  0x2a
#define RSHIFT  0x36
#define LMETA   0x38

#define LCTL    0x1d
#define CAPSL   0x3a


/* scan state flags */
#define INCTL           0x01    /* control key is down          */
#define INSHIFT         0x02    /* shift key is down            */
#define CAPSLOCK        0x04    /* caps lock mode               */
#define INMETA          0x08    /* meta (alt) key is down       */
#define EXTENDED        0x10    /* in extended character mode   */

#define EXTESC          0xe0    /* extended character escape    */
#define NOCHAR  256


/** kbtoa **/
unsigned int extchar( unsigned char code);
unsigned int kbtoa(unsigned char code);

#endif
