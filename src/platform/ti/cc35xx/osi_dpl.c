/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "FreeRTOS.h"
#include "ti/drivers/dpl/ClockP.h"
#include "ti/drivers/dpl/HwiP.h"
#include "ti/drivers/dpl/MessageQueueP.h"
#include "ti/drivers/dpl/MutexP.h"
#include "ti/drivers/dpl/SemaphoreP.h"
#include "ti/drivers/dpl/TaskP.h"
#include <osi_kernel.h>
#include <stdlib.h>

/*******************************************************************************

    DEFINES and SETTINGS

********************************************************************************/

#define TICK_PERIOD_nS (1000000000 / configTICK_RATE_HZ)
#define TICK_PERIOD_US (1000000 / configTICK_RATE_HZ)
/* System tick period in microseconds */
#define TICK_PERIOD_MS (1000 / configTICK_RATE_HZ)

#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
heap_debug_t heapdebug[HEAP_DBG_SIZE];
heap_debug_vport_t heapDebugVport[HEAP_DBG_VPORT_SIZE];
uint32_t dbg_offset         = 0;
uint32_t dbg_vport_offset   = 0;
uint32_t heapDbg_alloc_fail = 0;
int64_t heapDbg_total_alloc = 0;
#endif

// ClockP tick period, in microseconds convert to mSEC
#define ClockP_TICK_PERIOD_MS (ClockP_getSystemTickPeriod() * 1000)

/*******************************************************************************

    MISCELLANEOUS

********************************************************************************/
/*!
    \brief  convert tick to msec
*/
uint32_t TICK_TO_mSEC(uint32_t Tick)
{
    return ((Tick) / ClockP_TICK_PERIOD_MS);
}

/*!
    \brief  convert msec to tick
*/
uint32_t mSEC_TO_TICK(uint32_t mSec)
{
    return ((mSec) * (ClockP_TICK_PERIOD_MS));
}

/*!
    \brief  assert function for the upper mac library

    \param  condition   -   if TRUE, assert will occur

    \return
    \note
    \warning
*/

/*******************************************************************************

    Critical section

********************************************************************************/

/*!
    \brief  This function use to entering into critical section. It is provide a basic critical section implementation that works
     by simply disabling interrupts.
    \param  void
    \return - void
    \note
    \warning
*/
uint32_t osi_EnterCritical(void)
{
    uint32_t ulKey;

    ulKey = (uint32_t) HwiP_disable();
    TaskP_disableScheduler();
    return ulKey;
}

/*!
    \brief  This function use to exit critical section
    \param  void
    \return - void
    \note
    \warning
*/
uint32_t osi_ExitCritical(uint32_t ulKey)
{

    TaskP_restoreScheduler((uintptr_t) ulKey);
    HwiP_restore((uintptr_t) ulKey);
    return 0;
}

/*******************************************************************************

    SYNC

********************************************************************************/

/*!
        \brief 	This function creates a sync object

        The sync object is used for synchronization between different thread or ISR and
        a thread.

        \param	pSyncObj	-	pointer to the sync object control block

        \return upon successful creation the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_SyncObjCreate(OsiSyncObj_t * pSyncObj)
{
    // Check for NULL
    if (NULL == pSyncObj)
    {
        return OSI_INVALID_PARAMS;
    }
    SemaphoreP_Handle * pl_SyncObj = (SemaphoreP_Handle *) pSyncObj;

    *pl_SyncObj = SemaphoreP_createBinary(0);

    if ((SemaphoreP_Handle) (*pl_SyncObj) != NULL)
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function deletes a sync object

        \param	pSyncObj	-	pointer to the sync object control block

        \return upon successful deletion the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_SyncObjDelete(OsiSyncObj_t * pSyncObj)
{
    // Check for NULL
    if (NULL == pSyncObj)
    {
        return OSI_INVALID_PARAMS;
    }

    SemaphoreP_Handle * pl_SyncObj = (SemaphoreP_Handle *) pSyncObj;

    SemaphoreP_delete(*pl_SyncObj);
    return OSI_OK;
}

/*!
        \brief 		This function generates a sync signal for the object.

        All suspended threads waiting on this sync object are resumed

        \param		pSyncObj	-	pointer to the sync object control block

        \return 	upon successful signaling the function should return 0
                                Otherwise, a negative value indicating the error code shall be returned
        \note		the function could be called from ISR context
        \warning
*/
OsiReturnVal_e osi_SyncObjSignal(OsiSyncObj_t * pSyncObj)
{
    // Check for NULL
    if (NULL == pSyncObj)
    {
        return OSI_INVALID_PARAMS;
    }

    SemaphoreP_Handle * pl_SyncObj = (SemaphoreP_Handle *) pSyncObj;
    SemaphoreP_post(*pl_SyncObj);

    return OSI_OK;
}
/*!
        \brief 		This function generates a sync signal for the object
                                from ISR context.

        All suspended threads waiting on this sync object are resumed

        \param		pSyncObj	-	pointer to the sync object control block

        \return 	upon successful signalling the function should return 0
                                Otherwise, a negative value indicating the error code shall be returned
        \note		the function is called from ISR context
        \warning
*/
OsiReturnVal_e osi_SyncObjSignalFromISR(OsiSyncObj_t * pSyncObj)
{

    return osi_SyncObjSignal(pSyncObj);
}

/*!
        \brief 	This function waits for a sync signal of the specific sync object

        \param	pSyncObj	-	pointer to the sync object control block
        \param	Timeout		-	numeric value specifies the maximum number of mSec to
                                                        stay suspended while waiting for the sync signal
                                                        Currently, the simple link driver uses only two values:
                                                                - OSI_WAIT_FOREVER
                                                                - OSI_NO_WAIT

        \return upon successful reception of the signal within the timeout window return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_SyncObjWait(OsiSyncObj_t * pSyncObj, OsiTime_t Timeout)
{

    if (NULL == pSyncObj)
    {
        return OSI_INVALID_PARAMS;
    }
    // converting to ClockP Ticks
    if (Timeout != OSI_WAIT_FOREVER)
    {
        Timeout = (Timeout * 1000) / ClockP_getSystemTickPeriod();
    }

    SemaphoreP_Handle * pl_SyncObj = (SemaphoreP_Handle *) pSyncObj;
    if (SemaphoreP_OK == SemaphoreP_pend(*pl_SyncObj, (uint32_t) Timeout))
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function clears a sync object

        \param	pSyncObj	-	pointer to the sync object control block

        \return upon successful clearing the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_SyncObjClear(OsiSyncObj_t * pSyncObj)
{
    // Check for NULL
    if (NULL == pSyncObj)
    {
        return OSI_INVALID_PARAMS;
    }

    if (OSI_OK == osi_SyncObjWait(pSyncObj, 0))
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*******************************************************************************

    LOCK

********************************************************************************/
/*!
        \brief 	This function creates a locking object.

        The locking object is used for protecting a shared resources between different
        threads. Allow only one thread to access a section of code at a time.

        \param	pLockObj	-	pointer to the locking object control block

        \return upon successful creation the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_LockObjCreate(OsiLockObj_t * pLockObj)
{
    // Check for NULL
    if (NULL == pLockObj)
    {
        return OSI_INVALID_PARAMS;
    }
    MutexP_Params params;
    params.callback = NULL;

    *pLockObj = (OsiLockObj_t) MutexP_create(&params);
    if (*pLockObj != NULL)
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function deletes a locking object.

        \param	pLockObj	-	pointer to the locking object control block

        \return upon successful deletion the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_LockObjDelete(OsiLockObj_t * pLockObj)
{
    SemaphoreP_Handle * pl_LockObj = (SemaphoreP_Handle *) pLockObj;

    SemaphoreP_delete(*pl_LockObj);
    return OSI_OK;
}

/*!
        \brief 	This function locks a locking object.

        All other threads that call this function before this thread calls
        the osi_LockObjUnlock would be suspended

        \param	pLockObj	-	pointer to the locking object control block
        \param	Timeout		-	numeric value specifies the maximum number of mSec to
                                                        stay suspended while waiting for the locking object
                                                        Currently, the simple link driver uses only two values:
                                                                - OSI_WAIT_FOREVER
                                                                - OSI_NO_WAIT


        \return upon successful reception of the locking object the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_LockObjLock(OsiLockObj_t * pLockObj, OsiTime_t Timeout)
{
    // Check for NULL
    if (NULL == pLockObj)
    {
        return OSI_INVALID_PARAMS;
    }
    // converting to ClockP Ticks
    if (Timeout != OSI_WAIT_FOREVER)
    {
        Timeout = (Timeout * 1000) / ClockP_getSystemTickPeriod();
    }

    SemaphoreP_Handle * pl_LockObj = (SemaphoreP_Handle *) pLockObj;
    if (SemaphoreP_OK == SemaphoreP_pend(*pl_LockObj, (uint32_t) Timeout))
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function unlock a locking object.

        \param	pLockObj	-	pointer to the locking object control block

        \return upon successful unlocking the function should return 0
                        Otherwise, a negative value indicating the error code shall be returned
        \note
        \warning
*/
OsiReturnVal_e osi_LockObjUnlock(OsiLockObj_t * pLockObj)
{
    // Check for NULL
    if (NULL == pLockObj)
    {
        return OSI_INVALID_PARAMS;
    }
    // Release Semaphore
    SemaphoreP_Handle * pl_LockObj = (SemaphoreP_Handle *) pLockObj;
    SemaphoreP_post(*pl_LockObj);

    return OSI_OK;
}

/*******************************************************************************

    MESSAGE QUEUE

********************************************************************************/
/*!
        \brief 	This function is used to create the MsgQ

        \param	pMsgQ	-	pointer to the message queue
        \param	pMsgQName	-	msg queue name
        \param	MsgSize	-	size of message on the queue
        \param	MaxMsgs	-	max. number of msgs that the queue can hold

        \return - OsiReturnVal_e
        \note
        \warning
*/
OsiReturnVal_e osi_MsgQCreate(OsiMsgQ_t * pMsgQ, char * pMsgQName, uint32_t MsgSize, uint32_t MaxMsgs)
{
    // Check for NULL
    if (NULL == pMsgQ)
    {
        return OSI_INVALID_PARAMS;
    }

    MessageQueueP_Handle handle = 0;

    // Create Queue
    handle = MessageQueueP_create((size_t) MsgSize, (size_t) MaxMsgs);
    if (handle == 0)
    {
        return OSI_OPERATION_FAILED;
    }

    *pMsgQ = (OsiMsgQ_t) handle;
    if ((*pMsgQ) != NULL)
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function is used to delete the MsgQ

        \param	pMsgQ	-	pointer to the message queue

        \return - OsiReturnVal_e
        \note
        \warning
*/
OsiReturnVal_e osi_MsgQDelete(OsiMsgQ_t * pMsgQ)
{
    // Check for NULL
    if (NULL == pMsgQ)
    {
        return OSI_INVALID_PARAMS;
    }
    MessageQueueP_delete((MessageQueueP_Handle) *pMsgQ);
    return OSI_OK;
}

/*!
        \brief 	This function is used to write data to the MsgQ

        \param	pMsgQ	-	pointer to the message queue
        \param	pMsg	-	pointer to the Msg strut to read into
        \param	Timeout	-	timeout to wait for the Msg to be available

        \return - OsiReturnVal_e
        \note
        \warning
*/
OsiReturnVal_e osi_MsgQWrite(OsiMsgQ_t * pMsgQ, void * pMsg, OsiTime_t Timeout, uint8_t flags)
{
    // Check for NULL
    if (NULL == pMsgQ)
    {
        return OSI_INVALID_PARAMS;
    }

    if (MessageQueueP_OK == MessageQueueP_post((MessageQueueP_Handle) *pMsgQ, pMsg, (uint32_t) Timeout))
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function is used to read data from the MsgQ

        \param	pMsgQ	-	pointer to the message queue
        \param	pMsg	-	pointer to the Msg strut to read into
        \param	Timeout	-	timeout to wait for the Msg to be available

        \return - OsiReturnVal_e
        \note
        \warning
*/
OsiReturnVal_e osi_MsgQRead(OsiMsgQ_t * pMsgQ, void * pMsg, OsiTime_t Timeout)
{
    BaseType_t status_MSG;
    // Check for NULL
    if (NULL == pMsgQ)
    {
        return OSI_INVALID_PARAMS;
    }

    if (Timeout == OSI_WAIT_FOREVER)
    {
        Timeout = portMAX_DELAY;
    }
    else
    {
        Timeout = Timeout * 1000;
    }
    status_MSG = MessageQueueP_pend((MessageQueueP_Handle) *pMsgQ, pMsg, (uint32_t) Timeout);

    if (status_MSG == MessageQueueP_OK)
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
        \brief 	This function returns the number of messages ready in the Queue.

        \param	pMsgQ	-	pointer to the message queue

        \return - The number of messages available in the queue.
        \note
        \warning
*/
uint32_t osi_MsgQCount(OsiMsgQ_t * pMsgQ)
{
    uint32_t Enqueued;

    if (NULL != pMsgQ)
    {
        Enqueued = MessageQueueP_getPendingCount((MessageQueueP_Handle) *pMsgQ);
        return Enqueued;
    }

    return 0;
}

/*!
        \brief 	This function returns whether there are any messages ready in the Queue.

        \param	pMsgQ	-	pointer to the message queue

        \return - BOOLEAN
        \note
        \warning
*/
BOOLEAN osi_MsgQIsEmpty(OsiMsgQ_t * pMsgQ)
{
    return (osi_MsgQCount(pMsgQ) == 0);
}

/*******************************************************************************

    Memory

********************************************************************************/

/*!
    \brief  Allocate dynamic memory

    \param  size

    \return - ptr to buffer or NULL
    \note
    \warning
*/
#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
volatile INT32 totalloc = 0;
#endif
void * os_malloc(size_t size)
{
    uintptr_t sus_key;
    void * ptr = NULL;
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
    uint32_t local_dbg_offset, lr;
    lr      = __get_LR();
    sus_key = TaskP_disableScheduler();
    dbg_offset++;
    dbg_offset       = dbg_offset % HEAP_DBG_SIZE;
    local_dbg_offset = dbg_offset;
    TaskP_restoreScheduler(sus_key);
    heapdebug[local_dbg_offset].isFree    = 0;
    heapdebug[local_dbg_offset].isUsed    = 1;
    heapdebug[local_dbg_offset].isfinshed = 0;
    heapdebug[local_dbg_offset].ptr       = 0;
    heapdebug[local_dbg_offset].funcId    = 1;
    heapdebug[local_dbg_offset].lr        = lr;
    heapdebug[local_dbg_offset].allocFail = 0;
    heapdebug[local_dbg_offset].allocFail = 0;
    heapdebug[local_dbg_offset].size      = size;
#endif
#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++MALLOC  LR 0x%x %d++++++++++", __get_LR(), size);
#endif // PRINT_DBG_MALLOC_FREE
    if (0 != size)
    {
        sus_key = TaskP_disableScheduler();
        ptr     = malloc(size);
        TaskP_restoreScheduler(sus_key);
    }
#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++MALLOC = 0x%x ++++++++++", (uint32_t) ptr);
#endif // PRINT_DBG_MALLOC_FREE

#ifdef ASSERT_ON_MALLOC_FAIL
    if (!ptr)
    {
        assert(0);
    }
#endif

#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
    sus_key  = TaskP_disableScheduler();
    totalloc = totalloc + (uint32_t) (*((uint32_t *) ptr - 2) - 1);
    TaskP_restoreScheduler(sus_key);
    // Report("+++++++++++++ os_malloc total_allloc = %d\n\r", totalloc);
#endif
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
    heapdebug[local_dbg_offset].isfinshed = 1;
    heapdebug[local_dbg_offset].ptr       = (uint32_t) ptr;
    if (!ptr)
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_alloc_fail++;
        TaskP_restoreScheduler(sus_key);
        heapdebug[local_dbg_offset].allocFail = 1;
    }
    else
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_total_alloc += (uint32_t) (*((uint32_t *) ptr - 2) - 1) + 8; //(8 overhead)
        TaskP_restoreScheduler(sus_key);
    }
#endif
    return ptr;
}

/*!
    \brief  Re-allocate dynamic memory

    \param  ptr Old buffer from os_malloc() or os_realloc()
    \param  newsize of the new buffer

    \return - Allocated buffer or %NULL on failure
    \note
    \warning -  Caller is responsible for freeing the returned buffer with os_free().
                If re-allocation fails, %NULL is returned and the original buffer (ptr) is
                not freed and caller is still responsible for freeing it.
*/
void * os_realloc(void * ptr, size_t newsize)
{
    uintptr_t sus_key;
    void * ptrnew = NULL;
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
    uint32_t local_dbg_offset, lr, sizeToRemove = 0;
    lr      = __get_LR();
    sus_key = TaskP_disableScheduler();
    dbg_offset++;
    dbg_offset       = dbg_offset % HEAP_DBG_SIZE;
    local_dbg_offset = dbg_offset;
    TaskP_restoreScheduler(sus_key);
    heapdebug[local_dbg_offset].isFree    = 0;
    heapdebug[local_dbg_offset].isUsed    = 1;
    heapdebug[local_dbg_offset].isfinshed = 0;
    heapdebug[local_dbg_offset].lr        = lr;
    heapdebug[local_dbg_offset].ptr       = 0; // (uint32_t)ptr;
    heapdebug[local_dbg_offset].funcId    = 2;
    heapdebug[local_dbg_offset].allocFail = 0;
    heapdebug[local_dbg_offset].ptrOld    = (uint32_t) ptr;
    if (ptr)
    {
        heapdebug[local_dbg_offset].size = (uint32_t) (*((uint32_t *) ptr - 2) - 1);
        sizeToRemove                     = (uint32_t) (*((uint32_t *) ptr - 2) - 1) + 8;
    }
#endif

#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++REALLOC  LR 0x%x ++++++++++", __get_LR());
#endif // PRINT_DBG_MALLOC_FREE
    if (0 != newsize)
    {
#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
        if (ptr)
        {
            vsus_key = TaskP_disableScheduler();
            totalloc = totalloc - (uint32_t) (*((uint32_t *) ptr - 2) - 1);
            TaskP_restoreScheduler(sus_key);
        }
#endif
        sus_key = TaskP_disableScheduler();
        ptrnew  = realloc(ptr, newsize);
        TaskP_restoreScheduler(sus_key);
    }
#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++REALLOC old ptr 0x%x newptr = 0x%x ++++++++++", (uint32_t) ptr, ptrnew);
#endif // PRINT_DBG_MALLOC_FREE

#ifdef ASSERT_ON_MALLOC_FAIL
    if (!ptrnew)
    {
        assert(0);
    }
#endif
#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
    sus_key  = TaskP_disableScheduler();
    totalloc = totalloc + (uint32_t) (*((uint32_t *) ptrnew - 2) - 1);
    TaskP_restoreScheduler(sus_key);
    // Report("+++++++++++++ os_realloc total_alloc = %d\n\r", totalloc);
#endif
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
    heapdebug[local_dbg_offset].isfinshed = 1;
    heapdebug[local_dbg_offset].ptr       = (uint32_t) ptrnew;
    heapdebug[local_dbg_offset].size      = newsize;
    if (!ptrnew)
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_alloc_fail++;
        TaskP_restoreScheduler(sus_key);
        heapdebug[local_dbg_offset].allocFail = 1;
    }
    else
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_total_alloc += ((uint32_t) (*((uint32_t *) ptrnew - 2))) + 8 - sizeToRemove; //(8 overhead)
        TaskP_restoreScheduler(sus_key);
    }
#endif

    return ptrnew;
}

/*!
    \brief  Re-Allocate and zero memory blocks

    \param  ptr to old array
    \param  nmemb number of blocks to allocate
    \param  size size of each block in bytes

    \return - Allocated buffer or %NULL on failure
    \note
*/
void * os_realloc_array(void * ptr, size_t nmemb, size_t size)
{
    if (size && nmemb > (~(size_t) 0) / size)
        return NULL;
    return os_realloc(ptr, nmemb * size);
}

/*!
    \brief  Allocate and zero memory blocks

    \param  nmemb number of blocks to allocate
    \param  size size of each block in bytes

    \return - Allocated buffer or %NULL on failure
    \note
*/
void * os_calloc(size_t nmemb, size_t size)
{
    uintptr_t sus_key;
    void * ptr = NULL;

#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
    uint32_t local_dbg_offset, lr;
    lr      = __get_LR();
    sus_key = TaskP_disableScheduler();
    dbg_offset++;
    dbg_offset       = dbg_offset % HEAP_DBG_SIZE;
    local_dbg_offset = dbg_offset;
    TaskP_restoreScheduler(sus_key);
    heapdebug[local_dbg_offset].isFree    = 0;
    heapdebug[local_dbg_offset].isUsed    = 1;
    heapdebug[local_dbg_offset].isfinshed = 0;
    heapdebug[local_dbg_offset].lr        = lr;
    heapdebug[local_dbg_offset].size      = size;
    heapdebug[local_dbg_offset].ptr       = 0; //(uint32_t)ptr;
    heapdebug[local_dbg_offset].funcId    = 3;
    heapdebug[local_dbg_offset].allocFail = 0;
    heapdebug[local_dbg_offset].size      = size;
#endif

#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++CALLOC  LR 0x%x ++++++++++", __get_LR());
#endif // PRINT_DBG_MALLOC_FREE
    if (0 != size)
    {
        sus_key = TaskP_disableScheduler();
        ptr     = calloc(nmemb, size);
        TaskP_restoreScheduler(sus_key);
    }

#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++CALLOC = 0x%x ++++++++++", (uint32_t) ptr);
#endif
#ifdef ASSERT_ON_MALLOC_FAIL
    if (!ptr)
    {
        assert(0);
    }
#endif
#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
    sus_key  = TaskP_disableScheduler();
    totalloc = totalloc + (uint32_t) (*((uint32_t *) ptr - 2) - 1);
    TaskP_restoreScheduler(sus_key);
    // Report("+++++++++++++ os_calloc total_alloc = %d\n\r", totalloc);
#endif
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17 //OSPREY_MX-17
    heapdebug[local_dbg_offset].isfinshed = 1;
    heapdebug[local_dbg_offset].ptr       = (uint32_t) ptr;
    if (!ptr)
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_alloc_fail++;
        TaskP_restoreScheduler(sus_key);
        heapdebug[local_dbg_offset].allocFail = 1;
    }
    else
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_total_alloc += (uint32_t) (*((uint32_t *) ptr - 2) - 1) + 8;
        TaskP_restoreScheduler(sus_key);
    }
#endif

    return ptr;
}

/*!
    \brief  Allocate and zero memory

    \param  size - in bytes

    \return - Allocated buffer or %NULL on failure
    \note
*/
void * os_zalloc(size_t size)
{
    uintptr_t sus_key;
    void * ptr = NULL;
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17 //OSPREY_MX-17
    uint32_t local_dbg_offset, lr;
    lr      = __get_LR();
    sus_key = TaskP_disableScheduler();
    dbg_offset++;
    dbg_offset       = dbg_offset % HEAP_DBG_SIZE;
    local_dbg_offset = dbg_offset;
    TaskP_restoreScheduler(sus_key);
    heapdebug[local_dbg_offset].isFree    = 0;
    heapdebug[local_dbg_offset].isUsed    = 1;
    heapdebug[local_dbg_offset].isfinshed = 0;
    heapdebug[local_dbg_offset].lr        = lr;
    heapdebug[local_dbg_offset].size      = size;
    heapdebug[local_dbg_offset].ptr       = 0; //(uint32_t)ptr;
    heapdebug[local_dbg_offset].funcId    = 4;
    heapdebug[local_dbg_offset].allocFail = 0;
#endif

#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++ZALLOC  LR 0x%x  %d ++++++++++", __get_LR(), size);
#endif //
    if (0 != size)
    {
        sus_key = TaskP_disableScheduler();
        ptr     = calloc(1, size);
        TaskP_restoreScheduler(sus_key);
    }
#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++ZALLOC = 0x%x ++++++++++", (uint32_t) ptr);
#endif
#ifdef ASSERT_ON_MALLOC_FAIL
    if (!ptr)
    {
        assert(0);
    }
#endif
#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
    sus_key  = TaskP_disableScheduler();
    totalloc = totalloc + (uint32_t) (*((uint32_t *) ptr - 2) - 1);
    TaskP_restoreScheduler(sus_key);
    Report("\n\r+++++++++++++ os_malloc total_alloc = %d", totalloc);
#endif
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17 //OSPREY_MX-17
    heapdebug[local_dbg_offset].isfinshed = 1;
    heapdebug[local_dbg_offset].ptr       = (uint32_t) ptr;
    if (!ptr)
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_alloc_fail++;
        TaskP_restoreScheduler(sus_key);
        heapdebug[local_dbg_offset].allocFail = 1;
    }
    else
    {
        sus_key = TaskP_disableScheduler();
        heapDbg_total_alloc += (uint32_t) (*((uint32_t *) ptr - 2) - 1) + 8; //(8 overhead)
        TaskP_restoreScheduler(sus_key);
    }
#endif
    return ptr;
}

/*!
    \brief  Free memory

    \param  ptr - pointer to release

    \return
    \note
*/
void os_free(void * ptr)
{
    uintptr_t sus_key;
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17 //OSPREY_MX-17
    uint32_t local_dbg_offset, lr;
#endif
    if (!ptr)
        return;
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17 //OSPREY_MX-17
    lr      = __get_LR();
    sus_key = TaskP_disableScheduler();
    dbg_offset++;
    dbg_offset       = dbg_offset % HEAP_DBG_SIZE;
    local_dbg_offset = dbg_offset;
    heapDbg_total_alloc -= (uint32_t) (*((uint32_t *) ptr - 2) - 1) + 8; //(8 overhead)
    TaskP_restoreScheduler(sus_key);
    heapdebug[local_dbg_offset].isFree    = 1;
    heapdebug[local_dbg_offset].isUsed    = 1;
    heapdebug[local_dbg_offset].isfinshed = 0;
    heapdebug[local_dbg_offset].size      = (uint32_t) (*((uint32_t *) ptr - 2) - 1);
    heapdebug[local_dbg_offset].ptr       = (uint32_t) ptr;
    heapdebug[local_dbg_offset].funcId    = 5;
    heapdebug[local_dbg_offset].allocFail = 0;
    heapdebug[local_dbg_offset].lr        = lr;
#endif
#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++FREE  LR 0x%x ++++++++++", __get_LR());
    Report("\n\r+++++++++++++FREE = 0x%x ++++++++++", (uint32_t) ptr);
#endif // PRINT_DBG_MALLOC_FREE

#ifdef PRINT_DBG_TOTAL_MALLOC_FREE
    sus_key  = TaskP_disableScheduler();
    totalloc = totalloc - (uint32_t) (*((uint32_t *) ptr - 2) - 1);
    TaskP_restoreScheduler(sus_key);
    if (totalloc < 0)
    {
        Report("\n\r+++++++++++++ 1 os_free total_alloc = %d", totalloc);
        assert(0);
    }
    else
    {
        Report("\n\r+++++++++++++ 2 os_free total_alloc = %d", totalloc);
    }
#endif
    sus_key = TaskP_disableScheduler();
    free(ptr);
    TaskP_restoreScheduler(sus_key);
#ifdef PRINT_DBG_MALLOC_FREE
    Report("\n\r+++++++++++++FREE DONE = 0x%x ++++++++++", (uint32_t) ptr);
#endif
#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17
    heapdebug[local_dbg_offset].isfinshed = 1;
#endif
}

#ifdef COLLECT_HEAP_DEBUG_INFO // OSPREY_MX-17

void os_printHeapdbg()
{
    int i;
    uint32_t local_dbg_offset;

    sus_key          = TaskP_disableScheduler();
    local_dbg_offset = dbg_offset;
    TaskP_restoreScheduler(sus_key);

    ReportNoLock("\n\rTotals alloc_fail:%d heapDbg_total_alloc:%d local_dbg_offset:%d", heapDbg_alloc_fail, heapDbg_total_alloc,
                 local_dbg_offset);

    i = (local_dbg_offset + 1);
    while (i != local_dbg_offset)
    {
        ReportNoLock("\n\r id:%d isUsed:%d funcId:%d isFree:%d isfinshed:%d allocFail:%d ptr:0x%x lr:0x%x size:%d ptrOld:%d", i,
                     heapdebug[i].isUsed, heapdebug[i].funcId, heapdebug[i].isFree, heapdebug[i].isfinshed, heapdebug[i].allocFail,
                     heapdebug[i].ptr, heapdebug[i].lr, heapdebug[i].size, heapdebug[i].ptrOld);
        i++;
        i = (i % HEAP_DBG_SIZE);
        // os_sleep(0,50000);
    }
    // last print for i == local_dbg_offset
    ReportNoLock("\n\r id:%d isUsed:%d funcId:%d isFree:%d isfinshed:%d allocFail:%d ptr:0x%x lr:0x%x size:%d ptrOld:%d", i,
                 heapdebug[i].isUsed, heapdebug[i].funcId, heapdebug[i].isFree, heapdebug[i].isfinshed, heapdebug[i].allocFail,
                 heapdebug[i].ptr, heapdebug[i].lr, heapdebug[i].size, heapdebug[i].ptrOld);

    ReportNoLock("\n\rTotals alloc_fail:%d heapDbg_total_alloc:%d local_dbg_offset:%d", heapDbg_alloc_fail, heapDbg_total_alloc,
                 dbg_offset);
}
#endif

/*******************************************************************************

    Clocks / Timers

********************************************************************************/
/*!
    \brief  Put the thread to sleep
    \param  sec
    \return - OSI_OK
    \note
    \warning
*/

OsiReturnVal_e osi_Sleep(OsiTime_t sec)
{

    ClockP_sleep(sec);
    return OSI_OK;
}

/*-----------------------------------------------------------*/

/*!
    \brief  Put the thread to sleep in micro seconds
    \param  usec - time in micro seconds
    \return - OSI_OK
    \note
    \warning
*/
OsiReturnVal_e osi_uSleep(OsiTime_t usec)
{

    ClockP_usleep(usec);
    return OSI_OK;
}

/*-----------------------------------------------------------*/
/*!
    \brief  Get free running time in mili seconds
    \param
    \return - time in mili seconds
    \note
    \warning
*/
uint32_t osi_GetTimeMS()
{
    static uint32_t prev_ticks = 0;
    static uint32_t msec       = 0;
    static uint32_t remainder  = 0;

    uint32_t current_ticks = (uint32_t) ClockP_getSystemTicks();
    uint32_t ticks_delta;
    uint32_t total_ticks;

    if (current_ticks < prev_ticks)
    {
        ticks_delta = (UINT32_MAX - prev_ticks + current_ticks);
    }
    else
    {
        ticks_delta = (current_ticks - prev_ticks);
    }

    total_ticks = ticks_delta + remainder;
    msec += total_ticks / ClockP_TICK_PERIOD_MS;
    remainder = total_ticks % ClockP_TICK_PERIOD_MS;

    prev_ticks = current_ticks;
    return msec;
}

/*******************************************************************************

    TIMERS

*******************************************************************************/

/*-----------------------------------------------------------*/

/*!
    \brief  Creates a timer in the OS
    \param pTimer - timer object preallocated allocated
    \param pTimerName - timer name
    \param pExpiryFunc - expiry callback function
    \param pParam - expiry callback parameters
    \return - OSI_xxx
    \note
    \warning
*/
OsiReturnVal_e osi_TimerCreate(OsiTimer_t * pTimer,
                               char * pTimerName, // NOT USED - remnant from freertos
                               P_TIMER_EXPIRY_LEGACY_FUNCTION pExpiryFunc, void * pParam)
{
    if ((NULL == pTimer) || (NULL == pExpiryFunc))
    {
        return OSI_INVALID_PARAMS;
    }

    ClockP_Fxn pl_ExpiryFunc = (ClockP_Fxn) pExpiryFunc;
    uintptr_t pl_Param       = (uintptr_t) pParam;

    ClockP_Params clock_params;
    clock_params.startFlag = 0; // will not start immediately
    clock_params.period    = 0xFFFFFFFF;
    clock_params.arg       = pl_Param;

    pTimer->osTimerHandler = ClockP_create(pl_ExpiryFunc, 0xFFFFFFFF, &clock_params);

    pTimer->callBackFunc = pExpiryFunc;
    pTimer->params       = pParam;

    if (NULL == pTimer->osTimerHandler)
    {
        return OSI_OPERATION_FAILED;
    }
    else
    {
        return OSI_OK;
    }
}

/*-----------------------------------------------------------*/

/*!
    \brief Deletes a timer in the OS
    \param pTimer - timer object
    \return - OSI_xxx
    \note
    \warning
*/
OsiReturnVal_e osi_TimerDelete(OsiTimer_t * pTimer)
{
    // uint32_t RetVal;

    if (NULL == pTimer)
    {
        return OSI_INVALID_PARAMS;
    }
    ClockP_delete(pTimer->osTimerHandler);

    return OSI_OK;
}

/*-----------------------------------------------------------*/

/*!
    \brief Start a created timer
    \param pTimer - timer object
    \param DurationMiliSec
    \param Periodic - is this timer periodic
    \return - OSI_xxx
    \note
    \warning
*/
OsiReturnVal_e osi_TimerStart(OsiTimer_t * pTimer, OsiTime_t DurationMiliSec, BOOLEAN Periodic)
{
    uint32_t DurationInTicks;

    // ensure that the timer is not running by calling to the deactivate function
    osi_TimerStop(pTimer);

    DurationInTicks = mSEC_TO_TICK(DurationMiliSec);
    ClockP_setTimeout(pTimer->osTimerHandler, DurationInTicks);
    ClockP_start(pTimer->osTimerHandler);
    return OSI_OK;
}

/*-----------------------------------------------------------*/
/*!
    \brief Stops a created timer
    \param pTimer - timer object
    \return - OSI_xxx
    \note
    \warning
*/
OsiReturnVal_e osi_TimerStop(OsiTimer_t * pTimer)
{
    // uint32_t RetVal;

    if (NULL == pTimer)
    {
        return OSI_INVALID_PARAMS;
    }

    ClockP_stop(pTimer->osTimerHandler);
    return OSI_OK;
}

/*!
    \brief  Check if given timer is active
    \param pTimer - timer object
    \return - OSI_xxx
    \note
    \warning
*/
OsiReturnVal_e osi_TimerIsActive(OsiTimer_t * pTimer)
{
    uint32_t RetVal;

    if (NULL == pTimer)
    {
        return OSI_INVALID_PARAMS;
    }

    RetVal = ClockP_isActive(pTimer->osTimerHandler);

    if (RetVal > 0)
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*

This function return the remaining duration untill the next expiry in mSec

Parameters:

    pTimer        -    pointer to the timer control block

*/
OsiTime_t osi_TimerGetRemainingTime(OsiTimer_t * pTimer)
{
    uint32_t RetVal;
    uint32_t RemainingTicks;
    OsiTime_t RemainingDuration = 0;

    if (NULL != pTimer)
    {
        RetVal = ClockP_getTimeout(pTimer->osTimerHandler);

        if (RetVal > 0)
        {
            RemainingTicks    = RetVal;
            RemainingDuration = TICK_TO_mSEC(RemainingTicks);
        }
    }

    return RemainingDuration;
}

/*******************************************************************************

    THREADS

********************************************************************************/
/*!
    \brief  Creates a thread in the OS
    \param pThread - return handler for the created thread
    \param pThreadName - thread name
    \param StackSize - stack size to be dynamically allocated inside the create function
    \param Priority - priority - 0 is lowest (IDLE) and (configMAX_PRIORITIES - 1) max
    \param pEntryFunc - thread entry function
    \param pParam - parameters to move to the thread entry function
    \return - OSI_xxx
    \note
    \warning
*/
OsiReturnVal_e osi_ThreadCreate(OsiThread_t * pThread, char * pThreadName, uint32_t StackSize, uint32_t Priority,
                                P_THREAD_ENTRY_FUNCTION pEntryFunc, void * pParam)
{
    // TaskP_Handle RetVal;
    TaskP_Params params;

    // it is possible to check that the thread is not created already using the
    // stack pointer in the OsiThread_t structure but for now it seems too
    // "tough". would be consider in the future...

    if ((NULL == pThread) || (StackSize < OSI_MIN_THREAD_STACK_SIZE) || (Priority >= 32) || (NULL == pEntryFunc))
    {
        return OSI_INVALID_PARAMS;
    }

    params.name      = pThreadName;
    params.arg       = pParam;
    params.priority  = (int) Priority;
    params.stackSize = (size_t) StackSize;
    params.stack     = NULL;

    // RetVal =
    *pThread = TaskP_create((TaskP_Function) pEntryFunc, &params);

    return OSI_OK;
}

/*!
    \brief  Deletes a thread in the OS
    \param pThread - return handler for the created thread
    \return - OSI_xxx
    \note
    \warning thread should to be in a safe code - while (1) loop for example
             in order to be deleted
*/
OsiReturnVal_e osi_ThreadDelete(OsiThread_t * pThread)
{
    if (NULL == pThread)
    {
        return OSI_INVALID_PARAMS;
    }

    TaskP_delete((TaskP_Handle) *pThread);

    return OSI_OK;
}

/*!
    \brief  Get currently running thread
    \param
    \return - currently running thread
    \note
    \warning
*/
OsiThread_t osi_GetCurrentThread()
{
    return (OsiThread_t) TaskP_getCurrentTask();
}

/*!
    \brief  Get the current  available heap size
    \param
    \return - currently running thread
    \note
    \warning
*/
size_t osi_GetFreeHeapSize()
{
    return (size_t) xPortGetFreeHeapSize();
}

/*******************************************************************************

    SEMAPHORE

********************************************************************************/

/*!
    \brief  This function creates a Semaphore object
            The Semaphore object is used for accounting resources which can be shared
            and obtained/released in different contexts, as well as having multicplicity of more than 1.

    \param  pSemaphoreObj        -   pointer to the Semaphore object (area allocated by the caller and remains in scope
                                                                      throughout the lifetime of the object).
            pSemaphoreObjName    -   pointer to the name of the semaphore object
            initialCount         -   count of the semaphore at creation.
                                     (the amount of resources available).
            maxCount             -   Max multiplicity of the semaphore.
                             Cannot release to a value which is larger than this value.

    \return upon successful unlocking the function should return 0
            Otherwise, a negative value indicating the error code shall be returned
*/
OsiReturnVal_e osi_SemaphoreObjCreate(OsiSemaphoreObj_t * pSemaphoreObj, const char * pSemaphoreObjName, const uint32 initialCount,
                                      const uint32 maxCount)
{
    // Check for NULL
    if (NULL == pSemaphoreObj)
    {
        return OSI_INVALID_PARAMS;
    }

    pSemaphoreObj->max_count = maxCount;

    SemaphoreP_Handle handle = 0;

    SemaphoreP_Params params;
    params.mode     = SemaphoreP_Mode_COUNTING;
    params.callback = NULL;

    handle = SemaphoreP_create((unsigned int) initialCount, &params);

    pSemaphoreObj->Semaphore = handle;
    if ((SemaphoreP_Handle) (pSemaphoreObj->Semaphore) != NULL)
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
    \brief This function deletes a Semaphore object.
           After this function is called, the area holding the semaphore objetc is no longer
           in used and can be reused.

    \param  pSemaphoreObj        -   pointer to the semaphore object to release

    \return upon successful unlocking the function should return 0
            Otherwise, a negative value indicating the error code shall be returned
*/
OsiReturnVal_e osi_SemaphoreObjDelete(OsiSemaphoreObj_t * pSemaphoreObj)
{
    // Check for NULL
    if (NULL == pSemaphoreObj)
    {
        return OSI_INVALID_PARAMS;
    }
    SemaphoreP_Handle * pl_SemaphoreObj = (SemaphoreP_Handle *) pSemaphoreObj->Semaphore;
    SemaphoreP_delete(*pl_SemaphoreObj);
    return OSI_OK;
}

/*!
    \brief This function attempts to obtain a semaphore instance.
           If no instance is available, it waits for the designated timeout.

    \param pSemaphoreObj   -   pointer semaphore object to obtain instance of.
           Timeout         -   numeric value specifies the maximum number of mSec to
                               stay suspended while waiting for an instance to become available.
    \return upon successful unlocking the function should return 0
            Otherwise, a negative value indicating the error code shall be returned
*/
OsiReturnVal_e osi_SemaphoreObjObtain(OsiSemaphoreObj_t * pSemaphoreObj, OsiTime_t Timeout)
{

    if (OSI_WAIT_FOREVER != Timeout)
    {
        Timeout = (Timeout * 1000) / ClockP_getSystemTickPeriod();
    }
    // Check for NULL
    if (NULL == pSemaphoreObj)
    {
        return OSI_INVALID_PARAMS;
    }

    SemaphoreP_Handle * pl_SemaphoreObj = (SemaphoreP_Handle *) pSemaphoreObj->Semaphore;
    if (SemaphoreP_OK == SemaphoreP_pend(*pl_SemaphoreObj, (uint32_t) Timeout))
    {
        return OSI_OK;
    }
    else
    {
        return OSI_OPERATION_FAILED;
    }
}

/*!
    \brief This function releases an instance of a semaphore.
           Note that if the available multiplicity of the semaphore is already at its
           maximal value, this function would fail.

    \param pSemaphoreObj        -   pointer to the semaphore object to release.

    \return upon successful unlocking the function should return 0
            Otherwise, a negative value indicating the error code shall be returned
*/
OsiReturnVal_e osi_SemaphoreObjRelease(OsiSemaphoreObj_t * pSemaphoreObj)
{
    // Check for NULL
    if (NULL == pSemaphoreObj)
    {
        return OSI_INVALID_PARAMS;
    }

    SemaphoreP_Handle * pl_SemaphoreObj = (SemaphoreP_Handle *) pSemaphoreObj->Semaphore;
    SemaphoreP_post(*pl_SemaphoreObj);
    return OSI_OK;
}

/*!
    \brief This function returns the current count of an instance of a semaphore.
           Note that if the available multiplicity of the semaphore is already at its
           maximal value, this function would fail.

    \param pSemaphoreObj        -   pointer to the semaphore object to get count for.

    \return the function should return the count of a semaphore
*/
uint32_t osi_SemaphoreObjGetCount(OsiSemaphoreObj_t * pSemaphoreObj)
{
    // Check for NULL
    if (NULL == pSemaphoreObj)
    {
        return OSI_INVALID_PARAMS;
    }
    MessageQueueP_Handle * pl_SemaphoreObj = (MessageQueueP_Handle *) pSemaphoreObj->Semaphore;
    return MessageQueueP_getPendingCount(*pl_SemaphoreObj);
}
