/* memHeader.c : memory header linked list functions
 */
#include <i386.h>
#include <xeroskernel.h>

extern long freemem;		/* start of free mem (set in i386.c) */
extern char* maxaddr;		/* end of memory space (set in i386.c) */
static memHeader *rootHeader;	/* pointer to the first piece of free memory */

/*
 * Function:  initializeMemHeaderList 
 * --------------------
 * Initializes the free memory list and "allocates" the space occupied by
 *	the hole in memory.
 *
 *  @param: none
 *  @return: Returns OK if the memory initialization was successful 
 */
extern int initializeMemHeaderList( void ){
  //get the first address divisible by 16 after freemem
  memHeader* startingPoint = roundUpMemoryAddress((void *) freemem);

  //create a memory header at the first address divisible by 16
  //located within free memory
  rootHeader = startingPoint;
  rootHeader->size = HOLESTART - (int)&rootHeader->dataStart;
  rootHeader->prev = NULL;
  rootHeader->next = NULL;
  rootHeader->sanityCheck = (char *) &rootHeader->dataStart;

  //get the first address divisible by 16 after the hole
  startingPoint = roundUpMemoryAddress((void *) HOLEEND);

  //create a memory header at the first address divisible by 16
  //located within free memory after the hole
  memHeader * afterHoleHeader = startingPoint;
  
  rootHeader->next = (struct memHeader *) afterHoleHeader;

  afterHoleHeader->size = ((int)maxaddr + sizeof(char)) - (int)&afterHoleHeader->dataStart;
  afterHoleHeader->prev = (struct memHeader *) rootHeader;
  afterHoleHeader->next = NULL;
  afterHoleHeader->sanityCheck = (char *) &afterHoleHeader->dataStart;
  return OK;
}

/*
 * Function:  insertMemHeaderLink 
 * --------------------
 * Adds a given memory header to the list of free spaces in memory. A
 *	response will be returned to indicate if the operation was a
 *	success (OK) or a failure (SYSERR).
 *
 *  @param: A pointer to the memory header to be added to the free list 
 *  @return: The status of the operation
 */
extern int insertMemHeaderLink(memHeader* memPtr){
  if (!isValidMemoryAddress(memPtr)) {
  // Not allowed to allocate before memory section
    kprintf("mH:insertMHL:invalidMem:0x%x\n", memPtr);
    return SYSERR;
  }
  if (rootHeader == NULL){
    rootHeader = memPtr;
    return OK;
  }
  if (memPtr < rootHeader) {
    // append before, update rootHeader
    linkMemHeaderInFront(memPtr, rootHeader);
    rootHeader = memPtr;
    return OK;
  }
  memHeader* node = rootHeader;
  memHeader* prevNode;
  // iterate over nodes
  while (node) {
    if (node == memPtr) {
      // cannot allocate an existing pointer
      return SYSERR;
    }
    if (node > memPtr) {
      // insert memPtr in order
      linkMemHeaderInFront(memPtr, node);
      return OK;
    }
    prevNode = node;
    node = (memHeader *) node->next;
  }
  
  // Append node after last element of linked list
  linkMemHeaderBehind(prevNode, memPtr);
  return OK;
}

/*
 * Function:  linkMemHeaderInFront 
 * --------------------
 * Will insert the first memHeader parameter to be in place in front of the
 *	second memHeader. It will then join the appropriate next and prev
 *	links.
 *
 *  @param: Pointer to the memHeader to be inserted in front
 *  @param: Pointer to the memHeader currently in list 
 *  @return: The status of the operation (OK if success, SYSERR if else)
 */
extern int linkMemHeaderInFront(memHeader* newHeader, memHeader* memPtr) {
  newHeader->next = (struct memHeader*) memPtr;
  newHeader->prev = (struct memHeader*) memPtr->prev;
  if (memPtr->prev) {
    ((memHeader * ) memPtr->prev)->next = (struct memHeader*) newHeader;
  }
  memPtr->prev = (struct memHeader*) newHeader;
  return OK;
}

/*
 * Function:  linkMemHeaderBehind 
 * --------------------
 * Will insert the first memHeader parameter to be in place behind the
 *	second memHeader. It will then join the appropriate next and prev
 *	links.
 *
 *  @param: Pointer to the memHeader to be inserted behind
 *  @param: Pointer to the memHeader currently in list  
 *  @return: The status of the operation (OK if success, SYSERR if else)
 */
extern int linkMemHeaderBehind(memHeader* memPtr, memHeader* newHeader) {
  newHeader->prev = (struct memHeader*) memPtr;
  newHeader->next = (struct memHeader*) memPtr->next;
  if (memPtr->next) {
    ((memHeader * ) memPtr->next)->prev = (struct memHeader*) newHeader;
  }
  memPtr->next = (struct memHeader*) newHeader;
  return OK;
}

/*
 * Function:  isValidMemoryAddress 
 * --------------------
 * Ensures that allocations do not occur before the start of free memory,
 *	after the end of memory, and are divisible by 16. Returns a TRUE
 *	or FALSE to indicate validity 
 *
 *  @param: The address to be checked
 *  @return: Whether the memory location is a valid address space
 */
extern int isValidMemoryAddress(void *addr) {
  if ((long) addr < freemem) {
    return FALSE; 
  }
  if ((long) addr > (long) maxaddr) {
    return FALSE;
  }
  if ((long) addr % PAGE_SIZE != 0) {
    return FALSE;
  }
  return TRUE;
}

/*
 * Function:  roundUpMemoryAddress 
 * --------------------
 * Rounds up a given address if needed to be completely divisible
 *	by 16. This is done by using the PAGE_SIZE.
 *
 *  @param: An address to be rounded up
 *  @return: A valid address that is divisible by 16
 */
extern void *roundUpMemoryAddress(void *addr) {
  int remainder = ((long) addr) % PAGE_SIZE;
  if (remainder) {
    addr += (PAGE_SIZE - remainder);
  }
  return addr;
}

/*
 * Function:  getMemHeader 
 * --------------------
 * Returns a memHeader* to a free part of memory where memHeader.size >= sizeReq. If there
 *	is not space availible then NULL will be returned. A new head is created after the
 *	space allocated and is added to the free memory list.
 *
 *  @param: The size of memory requested (in bytes)
 *  @return: A pointer to a memory header with enough space
 */
extern memHeader *getMemHeader(size_t sizeReq) {
	memHeader *node = rootHeader;
	while (node){
		//if we find a block of memory that is big enough, use that header
		if(node->size >= sizeReq){
			
			//get the first address divisible by 16 after the area we are going to allocate
			//verify that this address isn't within the hole area or after freemem. If it is,
			//dont make a new memheader there
      			memHeader* startingPoint16 = roundUpMemoryAddress((node->dataStart) + sizeReq);
			int locStatus = 0x1;
			if( ((int) startingPoint16 >= HOLESTART-(sizeof(memHeader)) && ((int) startingPoint16 < HOLEEND-(sizeof(memHeader))))){
				locStatus = 0x2;		
			}else if((int) startingPoint16 >= (int) (maxaddr)){
				locStatus = 0x3;		
			}

			//create a new header after the area we allocate
			if(locStatus == 0x1){
				memHeader *newHeader = (memHeader *) startingPoint16;
				newHeader->size = (node->size) - (sizeof(memHeader) + (long) roundUpMemoryAddress((void *) sizeReq));
				newHeader->sanityCheck = (char *) &newHeader->dataStart;
				newHeader->prev = node->prev;
				newHeader->next = node->next;
			
			
				//check to see if we are changing the root
				if(node->prev == NULL){
				rootHeader = newHeader;
				} else {
				((memHeader*) node->prev)->next = (struct memHeader*) newHeader;
				}
				((memHeader*) node->next)->prev = (struct memHeader*) newHeader;
			} else {
				//just connect the previous node to the next node without creation
				if(node->prev == NULL){
					//make the nex node the root
					rootHeader = ((memHeader*) node->next);
					((memHeader*) node->next)->prev = NULL;
				} else if (locStatus==0x2){//Header in hole
					((memHeader*) node->next)->prev =  node->prev;
					((memHeader*) node->prev)->next =  node->next;
				} else if (locStatus==0x3){//Header in post
					if(node->prev == NULL){
						rootHeader = NULL;
					}
				}			
			}
			//Initialize the new node with NULL values and it's size
      			node->prev = NULL;
      			node->next = NULL;
      			node->size = (long) roundUpMemoryAddress((void *) sizeReq);

			//return allocated memory header
			return node;
		}
		
		node = (memHeader *) node->next;
	}
	return (void *) SYSERR;
}

/*
 * Function:  checkAndMergeMemHeaders 
 * --------------------
 * Checks to see if the space in both memory locations is contiguous, and
 *	if so will merge the second memory header and its space to the first
 *	memory header.
 *
 *  @param: The memory header with a lower address
 *  @param: The memory header with a higher address
 *  @return: The status of the operation (OK for success, FAILED for failure)
 */
extern int checkAndMergeMemHeaders(memHeader* former, memHeader* latter){
  void* endOfFormer = former->dataStart + former->size;
  if (roundUpMemoryAddress(endOfFormer) == latter) {
    // memHeaders are adjacent, merge
    // link pointers
    former->next = (struct memHeader*) latter->next;
    if (latter->next) {
      ((memHeader *) latter->next)->prev = (struct memHeader*) former;
    }
    latter->prev = NULL;
    latter->next = NULL;
    // resize the block of memory to represent the total memory of both blocks
    former->size = former->size + sizeof(memHeader) + latter->size;
    return OK;
  }
  return FAILED;
}

/*
 * Function:  printMemHeaders 
 * --------------------
 * Prints out the information for all of the currently free memory headers and
 *	returns the count of the headers.
 *
 *  @param: none
 *  @return: the number of headers of free memory
 */
extern int printMemHeaders( void ){
  //Print out the memory header at the lowest address
  if(rootHeader == NULL){return 0;}
  memHeader* node = rootHeader;
  printMemHeaderHelper(node);
  int count = 1;
  // Run through the current list of memory headers
  while (node->next) {
    node = (memHeader *) node->next;
    printMemHeaderHelper(node);
    count++;
  }
  kprintf("end. length: %d\n", count);
  return count;
};

/*
 * Function:  printMemHeaderHelper 
 * --------------------
 * Prints out the location, size, sanity check value, start of the data for the
 *	block of memory, address of the previous header, and address of the next
 *	header.
 *
 *  @param: A pointer to the memory header to be printed
 *  @return: none
 */
void printMemHeaderHelper(memHeader* memPtr){
  kprintf("memHeader: 0x%x "
    "  size:0x%x "
    "  sanityCheck:0x%x "
    "  dataStart:0x%x "
    "  prevAddr:0x%x "
    "  nextAddr:0x%x \n",\
    memPtr, memPtr->size, memPtr->sanityCheck, memPtr->dataStart,\
    memPtr->prev, memPtr->next);
}

/*
 * Function:  countMemHeaders 
 * --------------------
 * Counts the number of headers representing free memory.
 *
 *  @param: none
 *  @return: The number of memory headers for the free space
 */
extern int countMemHeaders( void ) {
  memHeader* node = rootHeader;
  int count = 1;
  while (node->next) {
    node = (memHeader *) node->next;
    count++;
  }
  return count;
}

/*
 * Function:  getRootHeader 
 * --------------------
 * A function to provide entry to the memHeder list.
 *
 *  @param: none
 *  @return: A pointer to the lowest address memory header
 */
extern memHeader *getRootHeader( void ) {
  // strictly for test purposes
  // use at your own risk
  return rootHeader;
}
