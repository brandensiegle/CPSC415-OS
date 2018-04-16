#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <tests.h>

static int passedAll;

extern int testKFree( void ) {
  // This function assumes that the state 
  // of memory is in the state provided by kinit
  passedAll = TRUE;
  char errorMsg[64];
  kprintf("\nBegin test\n");
  
  passedAll = testKFree1(errorMsg);
  printErrors(passedAll, errorMsg);
  passedAll = testKFree2(errorMsg);
  printErrors(passedAll, errorMsg);
  
  if (passedAll) {
    kprintf("\nAll tests passed ;)\n");
  }
  return 1;
}

int testKFree1(char *errorMsg) {
  int state;
  int count;
  int reqSize = 255;
  memHeader* prevRootHeader;
  memHeader* currRootHeader;
  void* dataStart;
  unsigned long prevRootHeaderSize;
  long sizeDiff;
  long addrDiff; 

  prevRootHeader = getRootHeader();
  prevRootHeaderSize = prevRootHeader->size;
  dataStart = kmalloc(reqSize);
  printMemHeaders();
  currRootHeader = getRootHeader();
  
  // check size param
  sizeDiff = prevRootHeaderSize - currRootHeader->size;
  addrDiff = (currRootHeader - prevRootHeader) * sizeof(memHeader);
  if (sizeDiff != addrDiff) {
    kprintf("addr & size dont match 0x%x, 0x%x\n", sizeDiff, addrDiff);
    strcpy(errorMsg, "addr & size do not match");
    return FALSE;
  }
  // check length of linked list
  count = countMemHeaders();
  if (count != 2) {
    strcpy(errorMsg, "expected 2 data");
    return FALSE;
  }
  // check that end of rootHeader is HOLESTART
  long endOfHeader = (long) currRootHeader->dataStart + currRootHeader->size;

  if( endOfHeader != HOLESTART){
    kprintf("holestart err 0x%x, 0x%x\n", endOfHeader, HOLESTART);
    strcpy(errorMsg, "end of rootHeader is not holestart");
    return FALSE;
  };

  // check addr of headers agree with reqSize
  void* expectedAddr = (void *) ((long) prevRootHeader) +  sizeof(memHeader) + reqSize;
  expectedAddr = roundUpMemoryAddress(expectedAddr);
  if(currRootHeader != expectedAddr) {
    kprintf("addr err 0x%x, 0x%x\n", currRootHeader, expectedAddr);
    strcpy(errorMsg, "new root header does not agree with reqSize");
    return FALSE;
  };

  // test KFree
  kprintf("kfree\n");
  state = kfree(dataStart);
  printMemHeaders();
  // kfree executed without errors
  if (!state) {
    strcpy(errorMsg, "kfree had an error\n");
    return FALSE;
  }

  // check length of memHeader
  count = countMemHeaders();
  if (count != 2) {
    strcpy(errorMsg, "expected 2 data");
    return FALSE;
  }

  // ensure header is the same as kinit
  currRootHeader = getRootHeader();
  if (currRootHeader->size != prevRootHeaderSize) {
    kprintf("kfree header size error: 0x%x, 0x%x\n", currRootHeader->size, prevRootHeaderSize);
    strcpy(errorMsg, "expected kfree header to have same size as original");
  }
  if (currRootHeader != prevRootHeader) {
     kprintf("kfree header addr error: 0x%x, 0x%x\n", currRootHeader, prevRootHeader);
    strcpy(errorMsg, "expected kfree header to have same addr as original");

  }
  return TRUE;
}

int testKFree2(char *errorMsg) {
  kprintf("Kfree test: merging\n");

  int i;
  int count;
  int reqSizes[3] = {472, 16004, 1541};
  long totalReqSize = 0;
  void* allocatedPtrs[3];
  int len = sizeof(reqSizes) / sizeof(int);
  long expectedAddr;
  long expectedSize;

  memHeader* origMemHeader;
  long origHeaderSize;
  memHeader* rootHeader;
  long rootHeaderSize;

  origMemHeader = getRootHeader();
  origHeaderSize = origMemHeader->size;
  // allocate three chunks of memory
  //i=0;
  for (i=0; i<len; i++) {
    allocatedPtrs[i] = kmalloc(reqSizes[i]);
    totalReqSize += (long) roundUpMemoryAddress((void *) reqSizes[i]);
  }
  printMemHeaders();
  rootHeader = getRootHeader();
  rootHeaderSize = rootHeader->size;
  // check if rootHeader is where its supposed to be
  expectedAddr = ((long) origMemHeader) + totalReqSize + (sizeof(memHeader) * len);
  if ((void*) expectedAddr != rootHeader) {
    kprintf("rootHeader addr err: 0x%x, 0x%x\n", expectedAddr, rootHeader);
    
    strcpy(errorMsg, "rootHeader addr error");
    return FALSE;
  }

  // free rightmost block, expect merge
  kfree(allocatedPtrs[2]);
  count = countMemHeaders();
  //kprintf("> merged right: ");
  //printMemHeaders();
  if (count != 2) {
    strcpy(errorMsg, "expected 2 data");
    return FALSE;
  }
  // check addr & size of merged root Header
  rootHeader = getRootHeader();
  if (backTrackMemHeader(allocatedPtrs[2]) != rootHeader) {
     kprintf("1merged rootHeader addr err: 0x%x, 0x%x\n", backTrackMemHeader(allocatedPtrs[2]), rootHeader);
    
    strcpy(errorMsg, "1merged rootHeader addr error");
    return FALSE;
  }
  expectedSize = rootHeaderSize + sizeof(memHeader) + (long) roundUpMemoryAddress((void *) reqSizes[2]);
  if (expectedSize != rootHeader->size) {
     kprintf("1merged rootHeader size err: 0x%x, 0x%x\n", expectedSize, rootHeader->size);
    
    strcpy(errorMsg, "1merged rootHeader addr error");
    return FALSE;
  }
  
  // merge leftmost header
  // free rightmost block, expect merge
  kfree(allocatedPtrs[0]);
  count = countMemHeaders();
  if (count != 3) {
    strcpy(errorMsg, "expected 3 data");
    return FALSE;
  }
   // check addr & size of merged root Header
  rootHeader = getRootHeader();
  rootHeaderSize = rootHeader->size;
  if (backTrackMemHeader(allocatedPtrs[0]) != rootHeader) {
     kprintf("2merged rootHeader addr err: 0x%x, 0x%x\n", backTrackMemHeader(allocatedPtrs[0]), rootHeader);
    
    strcpy(errorMsg, "2merged rootHeader addr error");
    return FALSE;
  }
  expectedSize =  (long) roundUpMemoryAddress((void *) reqSizes[0]);
  if (expectedSize != rootHeader->size) {
     kprintf("2merged rootHeader size err: 0x%x, 0x%x\n", expectedSize, rootHeader->size);
    
    strcpy(errorMsg, "2merged rootHeader addr error");
    return FALSE;
  }
  //kprintf("> merged left: ");
  //printMemHeaders();
  // kfree third & last section
  printMemHeaders();
  kfree(allocatedPtrs[1]);
  count = countMemHeaders();
  kprintf("> merged last: 0x%x",allocatedPtrs[1]);
  printMemHeaders();
  if (count != 2) {
    strcpy(errorMsg, "expected 2 data");
    return FALSE;
  }
  rootHeader = getRootHeader();
  rootHeaderSize = rootHeader->size;
  if (origMemHeader != rootHeader) {
    kprintf("3merged rootHeader addr err: 0x%x, 0x%x\n", origMemHeader, rootHeader);
    
    strcpy(errorMsg, "3merged rootHeader addr error");
    return FALSE;
  }
  if (origHeaderSize != rootHeaderSize) {
    kprintf("3merged rootHeader size err: 0x%x, 0x%x, ogptr:0x%x\n", origHeaderSize, rootHeaderSize, origMemHeader);
    
    strcpy(errorMsg, "3merged rootHeader size error");
    return FALSE;
  }
  return TRUE;
}


void printErrors(int state, char *errorMsg) {
  if (!state) {
    kprintf("Failed Test: %s\n", errorMsg);
  }
  return;
}
