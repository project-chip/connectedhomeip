/*
 * FreeRTOS Kernel V10.4.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/**
 ******************************************************************************
 * @file    stm32_mm.c
 * @author  MCD Application Team
 * @brief   Memory Manager Utility
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of https://www.FreeRTOS.org for more information.
 */
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if (configSUPPORT_DYNAMIC_ALLOCATION == 0)
#error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

#else
#include "stm32_mm.h"
#include "utilities_common.h"

#undef PRIVILEGED_DATA
#define PRIVILEGED_DATA

#undef PRIVILEGED_FUNCTION
#define PRIVILEGED_FUNCTION

#undef portBYTE_ALIGNMENT
#define portBYTE_ALIGNMENT (8)

#undef portBYTE_ALIGNMENT_MASK
#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK (0x0007)
#endif
#if portBYTE_ALIGNMENT == 4
#define portBYTE_ALIGNMENT_MASK (0x0003)
#endif

/* No test marker by default. */
#undef mtCOVERAGE_TEST_MARKER
#define mtCOVERAGE_TEST_MARKER()

#undef configASSERT
#define configASSERT(x)

/* No tracing by default. */
#undef traceMALLOC
#define traceMALLOC(pvReturn, xWantedSize)

#undef traceFREE
#define traceFREE(pvAddress, uiSize)

#undef vTaskSuspendAll
#define vTaskSuspendAll()

#undef xTaskResumeAll
#define xTaskResumeAll() (0)

#endif

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE ((size_t) (xHeapStructSize << 1))

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE ((size_t) 8)

#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
/* Allocate the memory for the heap. */
#if (configAPPLICATION_ALLOCATED_HEAP == 1)

/* The application writer has already defined the array used for the RTOS
 * heap - probably so it can be placed in a special segment or address. */
extern uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#else
PRIVILEGED_DATA static uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif /* configAPPLICATION_ALLOCATED_HEAP */
#endif

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
} BlockLink_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(BlockLink_t * pxBlockToInsert) PRIVILEGED_FUNCTION;

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
static void prvHeapInit(void) PRIVILEGED_FUNCTION;
#endif

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t xHeapStructSize =
    (sizeof(BlockLink_t) + ((size_t) (portBYTE_ALIGNMENT - 1))) & ~((size_t) portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
PRIVILEGED_DATA static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
PRIVILEGED_DATA static size_t xFreeBytesRemaining            = 0U;
PRIVILEGED_DATA static size_t xMinimumEverFreeBytesRemaining = 0U;
PRIVILEGED_DATA static size_t xNumberOfSuccessfulAllocations = 0;
PRIVILEGED_DATA static size_t xNumberOfSuccessfulFrees       = 0;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
 * member of an BlockLink_t structure is set then the block belongs to the
 * application.  When the bit is free the block is still part of the free heap
 * space. */
PRIVILEGED_DATA static size_t xBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS == 0)
void * UTIL_MM_GetBuffer(size_t xWantedSize)
#else
void * pvPortMalloc(size_t xWantedSize)
#endif
{
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void * pvReturn = NULL;

    vTaskSuspendAll();
    {
        /* If this is the first call to malloc then the heap will require
         * initialisation to setup the list of free blocks. */
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
        if (pxEnd == NULL)
        {
            prvHeapInit();
        }
        else
#endif
        {
            mtCOVERAGE_TEST_MARKER();
        }

        /* Check the requested block size is not so large that the top bit is
         * set.  The top bit of the block size member of the BlockLink_t structure
         * is used to determine who owns the block - the application or the
         * kernel, so it must be free. */
        if ((xWantedSize & xBlockAllocatedBit) == 0)
        {
            /* The wanted size is increased so it can contain a BlockLink_t
             * structure in addition to the requested amount of bytes. */
            if (xWantedSize > 0)
            {
                xWantedSize += xHeapStructSize;

                /* Ensure that blocks are always aligned to the required number
                 * of bytes. */
                if ((xWantedSize & portBYTE_ALIGNMENT_MASK) != 0x00)
                {
                    /* Byte alignment required. */
                    xWantedSize += (portBYTE_ALIGNMENT - (xWantedSize & portBYTE_ALIGNMENT_MASK));
                    configASSERT((xWantedSize & portBYTE_ALIGNMENT_MASK) == 0);
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }

            if ((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining))
            {
                /* Traverse the list from the start (lowest address) block until
                 * one of adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock         = xStart.pxNextFreeBlock;

                while ((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != NULL))
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock         = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached then a block of adequate size
                 * was not found. */
                if (pxBlock != pxEnd)
                {
                    /* Return the memory space pointed to - jumping over the
                     * BlockLink_t structure at its start. */
                    pvReturn = (void *) (((uint8_t *) pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                    /* This block is being returned for use so must be taken out
                     * of the list of free blocks. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* If the block is larger than required it can be split into
                     * two. */
                    if ((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE)
                    {
                        /* This block is to be split into two.  Create a new
                         * block following the number of bytes requested. The void
                         * cast is used to prevent byte alignment warnings from the
                         * compiler. */
                        pxNewBlockLink = (void *) (((uint8_t *) pxBlock) + xWantedSize);
                        configASSERT((((size_t) pxNewBlockLink) & portBYTE_ALIGNMENT_MASK) == 0);

                        /* Calculate the sizes of two blocks split from the
                         * single block. */
                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize        = xWantedSize;

                        /* Insert the new block into the list of free blocks. */
                        prvInsertBlockIntoFreeList(pxNewBlockLink);
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining)
                    {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }

                    /* The block is being returned - it is allocated and owned
                     * by the application and has no "next" block. */
                    pxBlock->xBlockSize |= xBlockAllocatedBit;
                    pxBlock->pxNextFreeBlock = NULL;
                    xNumberOfSuccessfulAllocations++;
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }

        traceMALLOC(pvReturn, xWantedSize);
    }
    (void) xTaskResumeAll();

#if (configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if (pvReturn == NULL)
        {
            extern void vApplicationMallocFailedHook(void);
            vApplicationMallocFailedHook();
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
#endif /* if ( configUSE_MALLOC_FAILED_HOOK == 1 ) */

    configASSERT((((size_t) pvReturn) & (size_t) portBYTE_ALIGNMENT_MASK) == 0);
    return pvReturn;
}
/*-----------------------------------------------------------*/
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS == 0)
void UTIL_MM_ReleaseBuffer(void * pv)
#else
void vPortFree(void * pv)
#endif
{
    uint8_t * puc = (uint8_t *) pv;
    BlockLink_t * pxLink;

    if (pv != NULL)
    {
        /* The memory being freed will have an BlockLink_t structure immediately
         * before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void *) puc;

        /* Check the block is actually allocated. */
        configASSERT((pxLink->xBlockSize & xBlockAllocatedBit) != 0);
        configASSERT(pxLink->pxNextFreeBlock == NULL);

        if ((pxLink->xBlockSize & xBlockAllocatedBit) != 0)
        {
            if (pxLink->pxNextFreeBlock == NULL)
            {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

                vTaskSuspendAll();
                {
                    /* Add this block to the list of free blocks. */
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    traceFREE(pv, pxLink->xBlockSize);
                    prvInsertBlockIntoFreeList(((BlockLink_t *) pxLink));
                    xNumberOfSuccessfulFrees++;
                }
                (void) xTaskResumeAll();
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
}
/*-----------------------------------------------------------*/
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
size_t xPortGetFreeHeapSize(void)
{
    return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize(void)
{
    return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks(void)
{
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/
static void prvHeapInit(void) /* PRIVILEGED_FUNCTION */
#else
void UTIL_MM_Init(uint8_t * p_pool, uint32_t pool_size)
#endif
{
    BlockLink_t * pxFirstFreeBlock;
    uint8_t * pucAlignedHeap;
    size_t uxAddress;
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
    size_t xTotalHeapSize = configTOTAL_HEAP_SIZE;
#else
    size_t xTotalHeapSize;
    xTotalHeapSize = pool_size;
#endif

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (size_t) p_pool;

    if ((uxAddress & portBYTE_ALIGNMENT_MASK) != 0)
    {
        uxAddress += (portBYTE_ALIGNMENT - 1);
        uxAddress &= ~((size_t) portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= uxAddress - (size_t) p_pool;
    }

    pucAlignedHeap = (uint8_t *) uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void *) pucAlignedHeap;
    xStart.xBlockSize      = (size_t) 0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    uxAddress = ((size_t) pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~((size_t) portBYTE_ALIGNMENT_MASK);
    pxEnd                  = (void *) uxAddress;
    pxEnd->xBlockSize      = 0;
    pxEnd->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock                  = (void *) pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize      = uxAddress - (size_t) pxFirstFreeBlock;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining            = pxFirstFreeBlock->xBlockSize;

    /* Work out the position of the top bit in a size_t variable. */
    xBlockAllocatedBit = ((size_t) 1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(BlockLink_t * pxBlockToInsert) /* PRIVILEGED_FUNCTION */
{
    BlockLink_t * pxIterator;
    uint8_t * puc;

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock)
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    puc = (uint8_t *) pxIterator;

    if ((puc + pxIterator->xBlockSize) == (uint8_t *) pxBlockToInsert)
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    puc = (uint8_t *) pxBlockToInsert;

    if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t *) pxIterator->pxNextFreeBlock)
    {
        if (pxIterator->pxNextFreeBlock != pxEnd)
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
     * before and the block after, then it's pxNextFreeBlock pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if (pxIterator != pxBlockToInsert)
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}
/*-----------------------------------------------------------*/
#if (KEEP_ORIGINAL_CODE_FROM_FREERTOS != 0)
void vPortGetHeapStats(HeapStats_t * pxHeapStats)
{
    BlockLink_t * pxBlock;
    size_t xBlocks = 0, xMaxSize = 0,
           xMinSize = portMAX_DELAY; /* portMAX_DELAY used as a portable way of getting the maximum value. */

    vTaskSuspendAll();
    {
        pxBlock = xStart.pxNextFreeBlock;

        /* pxBlock will be NULL if the heap has not been initialised.  The heap
         * is initialised automatically when the first allocation is made. */
        if (pxBlock != NULL)
        {
            do
            {
                /* Increment the number of blocks and record the largest block seen
                 * so far. */
                xBlocks++;

                if (pxBlock->xBlockSize > xMaxSize)
                {
                    xMaxSize = pxBlock->xBlockSize;
                }

                if (pxBlock->xBlockSize < xMinSize)
                {
                    xMinSize = pxBlock->xBlockSize;
                }

                /* Move to the next block in the chain until the last block is
                 * reached. */
                pxBlock = pxBlock->pxNextFreeBlock;
            } while (pxBlock != pxEnd);
        }
    }
    (void) xTaskResumeAll();

    pxHeapStats->xSizeOfLargestFreeBlockInBytes  = xMaxSize;
    pxHeapStats->xSizeOfSmallestFreeBlockInBytes = xMinSize;
    pxHeapStats->xNumberOfFreeBlocks             = xBlocks;

    taskENTER_CRITICAL();
    {
        pxHeapStats->xAvailableHeapSpaceInBytes     = xFreeBytesRemaining;
        pxHeapStats->xNumberOfSuccessfulAllocations = xNumberOfSuccessfulAllocations;
        pxHeapStats->xNumberOfSuccessfulFrees       = xNumberOfSuccessfulFrees;
        pxHeapStats->xMinimumEverFreeBytesRemaining = xMinimumEverFreeBytesRemaining;
    }
    taskEXIT_CRITICAL();
}
#endif
