#ifndef MEMTESTS_H
#define MEMTESTS_H

int     testKFree(void);
int     testKFree1(char *errorBuf);
int     testKFree2(char *errorBuf);
void    printErrors(int state, char *errorMsg);
void    initPids(void);

void    testIpc1(void);
void    testSend1(void);
void    testRecv1(void);

void    testIpc2(void);
void    testSend2(void);
void    testRecv2(void);
void    testIpc2p3(void);

void    testIpc3(void);
void    testIpc3p0(void);
void    testIpc3p1(void);

void    testIpc4(void);
void    testIpc4p0(void);
void    testIpc4p1(void);
void    testIpc4p2(void);
void    testIpc4p3(void);
void    testIpc4p4(void);

// test case 1a
void    testIpc5(void);
void    testIpc5p0(void);
void    testIpc5p1(void);

// test case 2a
void    testIpc6(void);
void    testIpc6p0(void);
void    testIpc6p1(void);
void    testIpc6p2(void);

// test case 2b
void    testIpc7(void);
void    testIpc7p0(void);
void    testIpc7p1(void);
void    testIpc7p2(void);
void    testIpc7p3(void);


// test case 3
void    testIpc8(void);
void    testIpc8p0(void);
void    testIpc8p1(void);


// test case 4a
void    testIpc9(void);
void    testIpc9p0(void);


// test case 4b
void    testIpc10(void);
void    testIpc10p0(void);
void    testIpc10p1(void);

// test getcputimes
void    testGCT_root(void);
void    testGCT_pc(void);

// test devices
void    testDev0(void);
void    testDev1(void);
void    testDev2(void);
void    testDev3(void);
void    testDev4(void);
void    testDev5(void);
void    testDev6(void);
void    testDev7(void);
void    testDev8(void);
#endif
