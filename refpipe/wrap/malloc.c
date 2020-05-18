/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 */

#include <stdlib.h>
#include <stdint.h>

#define configTOTAL_HEAP_SIZE	    ( ( size_t ) ( 0x2000 ) )

#define portBYTE_ALIGNMENT (sizeof(uintptr_t))
#define portBYTE_ALIGNMENT_MASK	( sizeof(uintptr_t) - 1 )
#define portPOINTER_SIZE_TYPE uintptr_t
typedef long BaseType_t;

/* A few bytes might be lost to byte aligning the heap start address. */
#define configADJUSTED_HEAP_SIZE	( configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT )

/*
 * Initialises the heap structures before their first use.
 */
static void prvHeapInit( void );

static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

/* Define the linked list structure.  This is used to link free blocks in order
   of their size. */
typedef struct A_BLOCK_LINK
{
  struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
  size_t xBlockSize;			/*<< The size of the free block. */
} BlockLink_t;


static const uint16_t heapSTRUCT_SIZE = ( ( sizeof ( BlockLink_t ) + ( portBYTE_ALIGNMENT - 1 ) ) & ~portBYTE_ALIGNMENT_MASK );
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( heapSTRUCT_SIZE * 2 ) )

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, xEnd;

/* Keeps track of the number of free bytes remaining, but says nothing about
   fragmentation. */
static size_t xFreeBytesRemaining = configADJUSTED_HEAP_SIZE;

/* STATIC FUNCTIONS ARE DEFINED AS MACROS TO MINIMIZE THE FUNCTION CALL DEPTH. */

/*
 * Insert a block into the list of free blocks - which is ordered by size of
 * the block.  Small blocks at the start of the list and large blocks at the end
 * of the list.
 */
#define prvInsertBlockIntoFreeList( pxBlockToInsert )			\
  {									\
    BlockLink_t *pxIterator;						\
    size_t xBlockSize;							\
									\
    xBlockSize = pxBlockToInsert->xBlockSize;				\
									\
    /* Iterate through the list until a block is found that has a larger size */ \
    /* than the block we are inserting. */				\
    for( pxIterator = &xStart; pxIterator->pxNextFreeBlock->xBlockSize < xBlockSize; pxIterator = pxIterator->pxNextFreeBlock )	\
      {									\
	/* There is nothing to do here - just iterate to the correct position. */ \
      }									\
									\
    /* Update the list to include the block being inserted in the correct */ \
    /* position. */							\
    pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;	\
    pxIterator->pxNextFreeBlock = pxBlockToInsert;			\
  }
/*-----------------------------------------------------------*/

#define MAX_COLORS 13
#if MAX_COLORS

// Special static variable used for applying tags to pointers returned from malloc
static void *volatile dover_ptr_tag[MAX_COLORS];
static uintptr_t dover_ptr_tag_index = 0;
#else
// Special static variable used for applying tags to pointers returned from malloc
static void *volatile dover_ptr_tag;
#endif
static void *volatile dover_ptr_untag;
static void *volatile dover_ptr_zero = 0;

// Convert bytes to words, rounding up
size_t btow(size_t bytes){
  uintptr_t words = bytes / sizeof(uintptr_t);
  // Handle fractional word
  if(bytes & (sizeof(uintptr_t) - 1))
    words++;
  return words;
}

// Strip tag from a pointer
void* __attribute__ ((noinline)) dover_remove_tag(void *volatile ptr) {
  void *volatile res;
  
  // remove tag from ptr
  dover_ptr_untag = ptr;
  res = dover_ptr_untag;

  return res;
}


// Apply tags to a block of heap memory by writing tagged 0's to the memory cells
//      Needs to be prevented from inlining and word aligned so propper 
//      code tags can be applied
void * __attribute__ ((noinline)) dover_tag(volatile uintptr_t *ptr, size_t bytes) {
  size_t words = btow(bytes);
  size_t count;
  volatile uintptr_t *p;
  void *volatile res;

#if MAX_COLORS
  // constrain index to bounds
  dover_ptr_tag_index++;
  dover_ptr_tag_index = dover_ptr_tag_index % MAX_COLORS;
  dover_ptr_tag[dover_ptr_tag_index] = (void*volatile)ptr;
  p = dover_ptr_tag[dover_ptr_tag_index];
  res = (void*volatile)p;
#else
  dover_ptr_tag = ptr;
  p = dover_ptr_tag;
  res = p;
#endif

  uintptr_t zero;

  zero = (uintptr_t)dover_ptr_zero;
  count = 0;
  //printk("do_tag %x\n", dover_remove_tag(p));
  while(count < words) {
    //printk("do_tag(%d) %d\n", count, p);
    *p = zero; // Tag the word
    p++;
    count++;
  }
  //printk("do_tag %x\n", dover_remove_tag(p));

  return res;
}

/*-----------------------------------------------------------*/

// Apply tags to a block of heap memory by writing tagged 0's to the memory cells
//      Needs to be prevented from inlining and word aligned so propper 
//      code tags can be applied
void __attribute__ ((noinline)) dover_untag(volatile uintptr_t *ptr, size_t bytes) {
  size_t words = btow(bytes);
  size_t count;
  volatile uintptr_t *p;
  uintptr_t zero;

  p = ptr;
  // grab a specially tagged zero
  zero = (uintptr_t)dover_ptr_zero;
  count = 0;
  while(count < words) {
    //printk("do_untag(%d) %d\n", count, p);
    *p = zero; // Tag the word
    p++;
    count++;
  }
}

/*-----------------------------------------------------------*/

void * pvPortMalloc( size_t xWantedSize )
{
  BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
  static BaseType_t xHeapHasBeenInitialised = 0;
  void *pvReturn = NULL;

  {
    /* If this is the first call to malloc then the heap will require
       initialisation to setup the list of free blocks. */
    if( xHeapHasBeenInitialised == 0 )
      {
	prvHeapInit();
	xHeapHasBeenInitialised = 1;
      }

    /* The wanted size is increased so it can contain a BlockLink_t
       structure in addition to the requested amount of bytes. */
    if( xWantedSize > 0 )
      {
	xWantedSize += heapSTRUCT_SIZE;

	/* Ensure that blocks are always aligned to the required number of bytes. */
	if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0 )
	  {
	    /* Byte alignment required. */
	    xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
	  }
      }

    if( ( xWantedSize > 0 ) && ( xWantedSize < configADJUSTED_HEAP_SIZE ) )
      {
	/* Blocks are stored in byte order - traverse the list from the start
	   (smallest) block until one of adequate size is found. */
	pxPreviousBlock = &xStart;
	pxBlock = xStart.pxNextFreeBlock;
	while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
	  {
	    pxPreviousBlock = pxBlock;
	    pxBlock = pxBlock->pxNextFreeBlock;
	  }

	/* If we found the end marker then a block of adequate size was not found. */
	if( pxBlock != &xEnd )
	  {
	    /* Return the memory space - jumping over the BlockLink_t structure
	       at its start. */
	    pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + heapSTRUCT_SIZE );

	    /* This block is being returned for use so must be taken out of the
	       list of free blocks. */
	    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

	    /* If the block is larger than required it can be split into two. */
	    if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
	      {
		/* This block is to be split into two.  Create a new block
		   following the number of bytes requested. The void cast is
		   used to prevent byte alignment warnings from the compiler. */
		pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );

		/* Calculate the sizes of two blocks split from the single
		   block. */
		pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
		pxBlock->xBlockSize = xWantedSize;

		/* Insert the new block into the list of free blocks. */
		prvInsertBlockIntoFreeList( ( pxNewBlockLink ) );
	      }

	    xFreeBytesRemaining -= pxBlock->xBlockSize;
	  }
      }

  }

  if(pvReturn)
    pvReturn = dover_tag(pvReturn, pxBlock->xBlockSize - heapSTRUCT_SIZE);
  //        else
  //          printk("malloc allocation failure, size = %d\n", xWantedSize);

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
  {
    if( pvReturn == NULL )
      {
	extern void vApplicationMallocFailedHook( void );
	vApplicationMallocFailedHook();
      }
  }
#endif

  return pvReturn;
}

void *__wrap_malloc( size_t xWantedSize ) {
  return pvPortMalloc(xWantedSize); 
}

/*-----------------------------------------------------------*/
void pvPortFree(void *pv) 
{
  uint8_t *puc;
  BlockLink_t *pxLink;


  if( pv != NULL )
    {

      puc = ( uint8_t * ) dover_remove_tag(pv);
      /* The memory being freed will have an BlockLink_t structure immediately
	 before it. */
      puc -= heapSTRUCT_SIZE;

      /* This unexpected casting is to keep some compilers from issuing
	 byte alignment warnings. */
      pxLink = ( void * ) puc;

      dover_untag(pv, pxLink->xBlockSize - heapSTRUCT_SIZE);

      {
	/* Add this block to the list of free blocks. */
	prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
	xFreeBytesRemaining += pxLink->xBlockSize;
      }
    }
}

void __wrap_free( void *pv ) {
  pvPortFree(pv);
}

/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
  return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
  /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit( void )
{
  BlockLink_t *pxFirstFreeBlock;
  uint8_t *pucAlignedHeap;

  /* Ensure the heap starts on a correctly aligned boundary. */
  pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE ) &ucHeap[ portBYTE_ALIGNMENT ] ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );

  /* xStart is used to hold a pointer to the first item in the list of free
     blocks.  The void cast is used to prevent compiler warnings. */
  xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
  xStart.xBlockSize = ( size_t ) 0;

  /* xEnd is used to mark the end of the list of free blocks. */
  xEnd.xBlockSize = configADJUSTED_HEAP_SIZE;
  xEnd.pxNextFreeBlock = NULL;

  /* To start with there is a single free block that is sized to take up the
     entire heap space. */
  pxFirstFreeBlock = ( void * ) pucAlignedHeap;
  pxFirstFreeBlock->xBlockSize = configADJUSTED_HEAP_SIZE;
  pxFirstFreeBlock->pxNextFreeBlock = &xEnd;
}
/*-----------------------------------------------------------*/
