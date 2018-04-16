/* mem.c : memory manager
 */

#include <xeroskernel.h>

/* Your code goes here */
/*
 * Function:  kmeminit 
 * --------------------
 * Initializes the free memory headers and prints out the state of free memory.
 *
 *  @param: none
 *  @return: none
 */
extern void kmeminit( void ){
	//kprintf("\n\nInitializing Memory\n");
  initializeMemHeaderList();
}


/*
 * Function:  kmalloc 
 * --------------------
 * Finds a free part of memory and returns a pointer to the first usable place of the
 *	allocated memory. Will return EMPTY if there is no more space to allocate.
 *
 *  @param: Size of memory to be allocated
 *  @return: A pointer to the location of memory availible for use
 */
extern void *kmalloc(size_t size){	
	//Find a memory header with enough space for allocation
	memHeader *allocatedMemHeader = getMemHeader(size);
	
	//check for error and print Error statement
	if (allocatedMemHeader == (void *) SYSERR) {
		return (void *) EMPTY;
	}
	//call insertMemHeaderLink(--) to add a memHeader
	return allocatedMemHeader->dataStart;
}


/*
 * Function:  kfree 
 * --------------------
 * Frees up the memory at the pointer provided for reuse by other processes. If
 *	the operation is successful the OK is returned, if it is not then SYSERR
 *	is returned.
 *
 *  @param: Location of the end of the memory header to be freed
 *  @return: The status of the operation
 */
extern int kfree(void *ptr){
  memHeader *memPtr;
  memHeader *precedingMemPtr;
  int isValidBlock;
  int isSuccess;
  //Find the location of the memheader based on the address provided
  memPtr = backTrackMemHeader(ptr);

  //Varify that the sanity check value is still valid
  isValidBlock = checkSanityCheck(memPtr->sanityCheck, ptr);
  if (! isValidBlock) {
    kprintf("Kfree has invalid block sc: 0x%x, ptr: 0x%x", memPtr->sanityCheck, ptr);
    return SYSERR;
  } 
  
  //Add the memory header back to the free memory list
  isSuccess = insertMemHeaderLink(memPtr);
  if (! isSuccess) {
    // error adding memPtr in KFree
    kprintf("Kfree error: insertMemLink not successful\n");
    return SYSERR;
  }

  // check for adjacent data blocks for potential merging
  precedingMemPtr = (memHeader *) memPtr->prev;
  // check in front of ptr
  isSuccess = checkAndMergeMemHeaders(precedingMemPtr, memPtr);
  if (isSuccess) {
    // if merge happened, memPtr is now merged with precedingMemPtr 
    memPtr = precedingMemPtr;
  }
  // check behind the ptr
  checkAndMergeMemHeaders(memPtr, (memHeader *) memPtr->next);
  return OK;
}

/*
 * Function:  backTrackMemHeader 
 * --------------------
 * Shifts the address back by the size of a memory header.
 *
 *  @param: Pointer to point at end of memory header
 *  @return: Pointer to pont at start of memory header
 */
memHeader *backTrackMemHeader(void *ptr){
  memHeader *memPtr = (memHeader *) (ptr - sizeof(memHeader));
  return memPtr;
}

/*
 * Function:  checkSanityCheck 
 * --------------------
 * Checks if the sanity check value of the memory header is correct.
 *
 *  @param: The location of the sanity check
 *  @param: The location of the start of memory block's data
 *  @return: Whether the sanity check is the correct value
 */
int checkSanityCheck(char *sanityCheck, void *ptr){
  if (sanityCheck == ptr){
    return TRUE;
  }
  return FALSE;
}

