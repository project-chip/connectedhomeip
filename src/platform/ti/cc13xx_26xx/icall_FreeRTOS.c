/******************************************************************************
 @file  icall_FreeRTOS.c

 @brief Indirect function Call dispatcher implementation on top of OS.

        This implementation uses heapmgr.h to implement a simple heap with low
        memory overhead but large processing overhead.<br>
        The size of the heap is determined with HEAPMGR_SIZE macro, which can
        be overridden with a compile option.
        Note: The ICall layer (e.g. this file) is using TI internal implementation of POSIX.
        For now, the ICall layer is not supports using outer POSIX on the application layer.

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2013-2024, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

#ifdef FREERTOS
#include "bget.h"
#include <FreeRTOS.h>
#include <task.h>
#endif

#ifdef FREERTOS
#include <queue.h>
#include <task.h>
#include <ti/drivers/dpl/ClockP.h>
#endif

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SwiP.h>
#define Hwi_disable HwiP_disable
#define Hwi_restore HwiP_restore
#define Hwi_disableinterrupt HwiP_disableInterrupt
#define Hwi_enableinterrupt HwiP_enableInterrupt
#define Swi_restore SwiP_restore
#define Swi_disable SwiP_disable
#define BIOS_WAIT_FOREVER (~(0U))
#define BIOS_NO_WAIT (0U)

#include "icall.h"
#include "icall_platform.h"
#include "osal.h"
#include <stdarg.h>
#include <stdint.h>

#ifndef ICALL_FEATURE_SEPARATE_IMGINFO
#include <icall_addrs.h>
#endif /* ICALL_FEATURE_SEPARATE_IMGINFO */

#ifndef Task_self
#define Task_self ICall_taskSelf
#endif

#ifndef ICALL_MAX_NUM_ENTITIES
/**
 * Maximum number of entities that use ICall, including service entities
 * and application entities.
 * The value may be overridden by a compile option.
 * Note that there are at least,
 * Primitive service, Stack services along with potentially generic
 * framework service for the stack thread.
 */
#define ICALL_MAX_NUM_ENTITIES 6
#endif

#ifndef ICALL_MAX_NUM_TASKS
/**
 * Maximum number of threads which include entities.
 * The value may be overridden by a compile option.
 */
#define ICALL_MAX_NUM_TASKS 2
#endif

/**
 * @internal
 * Service class value used to indicate an invalid (unused) entry
 */
#define ICALL_SERVICE_CLASS_INVALID_ENTRY 0x0000

/**
 * @internal
 * Service class value used to indicate an entry for an application entity
 */
#define ICALL_SERVICE_CLASS_APPLICATION ICALL_SERVICE_CLASS_MASK

/**
 * @internal
 * Primitive service entity ID
 */
#define ICALL_PRIMITIVE_ENTITY_ID 0

/**
 * @internal
 * Accessor macro to get a header field (next) from a message pointer
 */
#define ICALL_MSG_NEXT(_p) (((ICall_MsgHdr *) (_p) -1)->next)

/**
 * @internal
 * Accessor macro to get a header field (dest_id) from a message pointer
 */
#define ICALL_MSG_DEST_ID(_p) (((ICall_MsgHdr *) (_p) -1)->dest_id)

#ifndef ICALL_TIMER_TASK_STACK_SIZE
/**
 * @internal
 * Timer thread stack size
 */
#define ICALL_TIMER_TASK_STACK_SIZE (512)
#endif // ICALL_TIMER_TASK_STACK_SIZE

/**
 * @internal
 * Creation of the synchronous object between application and service
 */

#ifdef ICALL_EVENTS
#define ICALL_SYNC_HANDLE_CREATE() (Event_create(NULL, NULL))
#else /* ICALL_EVENTS */
#define ICALL_SYNC_HANDLE_CREATE() (Semaphore_create(0, NULL, NULL))
#endif /* ICALL_EVENTS */

/**
 * @internal
 * post the synchronous object between application and service
 */
#ifdef ICALL_EVENTS
#define ICALL_SYNC_HANDLE_POST(x) (Event_post(x, ICALL_MSG_EVENT_ID))
#define ICALL_SYNC_HANDLE_POST_WM(x) (Event_post(x, ICALL_WAITMATCH_EVENT_ID))
#else /* ICALL_EVENTS */
#define ICALL_SYNC_HANDLE_POST(x) (Semaphore_post(x))
#define ICALL_SYNC_HANDLE_POST_WM(x) (Semaphore_post(x)) /* Semaphore does not have event ID */
#endif                                                   /* ICALL_EVENTS */

/**
 * @internal
 * pend for the synchronous object between application and service
 */
#ifdef ICALL_EVENTS
#define ICALL_SYNC_HANDLE_PEND(x, t) (Event_pend(x, 0, ICALL_MSG_EVENT_ID, t))
#define ICALL_SYNC_HANDLE_PEND_WM(x, t) (Event_pend(x, 0, ICALL_WAITMATCH_EVENT_ID, t))
#else /* ICALL_EVENTS */
#define ICALL_SYNC_HANDLE_PEND(x, t) (Semaphore_pend(x, t))
#define ICALL_SYNC_HANDLE_PEND_WM(x, t) (Semaphore_pend(x, t)) /* Semaphore does not have event ID */
#endif                                                         /* ICALL_EVENTS */

/**
 * @internal
 * ticks
 */
#define CLOCK_TICKS_PERIOD (10)

/**
 * @internal
 * Data structure used to access critical section
 * state variable.
 * Without this data structure, C code will violate
 * C89 or C99 strict aliasing rule.
 */
typedef union _icall_cs_state_union_t
{
    /** critical section variable as declared in the interface */
    ICall_CSState state;
    /** @internal field used to access internal data */
    struct _icall_cs_state_aggr_t
    {
        /** field to store Swi_disable() return value */
        uint_least16_t swikey;
        /** field to store Hwi_disable() return value */
        uint_least16_t hwikey;
    } each;
} ICall_CSStateUnion;

/**
 * @internal Primitive service handler function type
 */
typedef ICall_Errno (*ICall_PrimSvcFunc)(ICall_FuncArgsHdr *);

#ifdef ICALL_FEATURE_SEPARATE_IMGINFO
/* Image information shall be in separate module */

/**
 * Array of entry function addresses of external images.
 *
 * Note that function address must be odd number for Thumb mode functions.
 */
extern const ICall_RemoteTaskEntry ICall_imgEntries[];
/**
 * Array of task priorities of external images.
 * One task is created per image to start off the image entry function.
 * Each element of this array correspond to the task priority of
 * each entry function defined in @ref ICall_imgEntries.
 */
extern const int ICall_imgTaskPriorities[];

/**
 * Array of task stack sizes of external images.
 * One task is created per image to start off the image entry function.
 * Each element of this array correspond to the task stack size of
 * each entry function defined in @ref ICall_imgEntries.
 */
extern const size_t ICall_imgTaskStackSizes[];

/**
 * Array of custom initialization parameters (pointers).
 * Each initialization parameter (pointer) is passed to each corresponding
 * image entry function defined in @ref ICall_imgEntries;
 */
extern const void * ICall_imgInitParams[];

/**
 * Number of external images.
 */
extern const uint_least8_t ICall_numImages;

#define icall_threadEntries ICall_imgEntries
#define ICall_threadPriorities ICall_imgTaskPriorities
#define ICall_threadStackSizes ICall_imgTaskStackSizes
#define ICall_getInitParams(_i) (ICall_imgInitParams[i])
#define ICALL_REMOTE_THREAD_COUNT ICall_numImages
#else /* ICALL_FEATURE_SEPARATE_IMGINFO */
/**
 * @internal
 * Array of entry function of external images.
 */
static const ICall_RemoteTaskEntry icall_threadEntries[] = ICALL_ADDR_MAPS;

/** @internal external image count */
#define ICALL_REMOTE_THREAD_COUNT (sizeof(icall_threadEntries) / sizeof(icall_threadEntries[0]))

/** @internal thread priorities to be assigned to each remote thread */
#ifndef BLE_STACK_TASK_PRIORITY
static const int ICall_threadPriorities[] = { ICALL_TASK_PRIORITIES };
#else
static const int ICall_threadPriorities[] = { BLE_STACK_TASK_PRIORITY };
#endif

/** @internal thread stack max depth for each remote thread */
static const size_t ICall_threadStackSizes[] = ICALL_TASK_STACK_SIZES;

/** @internal initialization parameter (pointer) for each remote thread */
#ifdef ICALL_CUSTOM_INIT_PARAMS
static const void * ICall_initParams[] = ICALL_CUSTOM_INIT_PARAMS;
#define ICall_getInitParams(_i) (ICall_initParams[i])
#else /* ICALL_CUSTOM_INIT_PARAMS */
#define ICall_getInitParams(_i) NULL
#endif /* ICALL_CUSTOM_INIT_PARAMS */

#endif /* ICALL_FEATURE_SEPARATE_IMGINFO */

/** @internal message queue */
typedef void * ICall_MsgQueue;

/** @internal data structure about a task using ICall module */
typedef struct _icall_task_entry_t
{
    TaskHandle_t task;
    ICall_SyncHandle syncHandle;
    ICall_MsgQueue queue;
} ICall_TaskEntry;

/** @internal data structure about an entity using ICall module */
typedef struct _icall_entity_entry_t
{
    ICall_ServiceEnum service;
    ICall_TaskEntry * task;
    ICall_ServiceFunc fn;
} ICall_entityEntry;

/** @internal storage to track all tasks using ICall module */
static ICall_TaskEntry ICall_tasks[ICALL_MAX_NUM_TASKS];

/** @internal storage to track all entities using ICall module */
static ICall_entityEntry ICall_entities[ICALL_MAX_NUM_ENTITIES];

#ifndef FREERTOS
extern mqd_t g_EventsQueueID;
#endif
/**
 * @internal
 * Wakeup schedule data structure definition
 */

#ifdef FREERTOS
void ICALL_Task_restore(UBaseType_t * OriginalParam);
void ICALL_Task_disable(UBaseType_t * OriginalParam);
#else

void ICALL_Task_restore(struct sched_param * OriginalParam);
void ICALL_Task_disable(struct sched_param * OriginalParam);
#endif
typedef struct _icall_schedule_t
{
    ClockP_Handle clockP;
    ICall_TimerCback cback;
    void * arg;
} ICall_ScheduleEntry;

/* For now critical sections completely disable hardware interrupts
 * because they are used from ISRs in MAC layer implementation.
 * If MAC layer implementation changes, critical section
 * implementation may change to reduce overall interrupt latency.
 */
/* Enter critical section implementation. See header file for comment. */
ICall_CSState ICall_enterCSImpl(void)
{

    ICall_CSStateUnion cu;
    cu.each.swikey = (uint_least16_t) Swi_disable();
    cu.each.hwikey = (uint_least16_t) Hwi_disable();
    return cu.state;
}
#ifdef FREERTOS
TaskHandle_t ICall_taskSelf(void)
#else
Task_Handle ICall_taskSelf(void)
#endif
{

    TaskHandle_t task = NULL;
#ifdef FREERTOS
    task = (TaskHandle_t) xTaskGetCurrentTaskHandle();
#else
    task = <handler need to be returned according to the chosen OS>;
#endif // FREERTOS
    return (task);
}

/* See header file for comment */
ICall_EnterCS ICall_enterCriticalSection = ICall_enterCSImpl;

/* leave critical section implementation. See header file for comment */
void ICall_leaveCSImpl(ICall_CSState key)
{
    ICall_CSStateUnion * cu = (ICall_CSStateUnion *) &key;
    Hwi_restore((uint32_t) cu->each.hwikey);
    Swi_restore((uint32_t) cu->each.swikey);
}

/* See header file for comment */
ICall_LeaveCS ICall_leaveCriticalSection = ICall_leaveCSImpl;

/* Implementing a simple heap using heapmgr.h template.
 * This simple heap depends on critical section implementation
 * and hence the template is used after critical section definition. */
void * ICall_heapMalloc(uint32_t size);
void * ICall_heapRealloc(void * blk, uint32_t size);
void ICall_heapFree(void * blk);

#define HEAPMGR_INIT ICall_heapInit
#define HEAPMGR_MALLOC ICall_heapMalloc
#define HEAPMGR_FREE ICall_heapFree
#define HEAPMGR_REALLOC ICall_heapRealloc
#define HEAPMGR_GETSTATS ICall_heapGetStats
#define HEAPMGR_MALLOC_LIMITED ICall_heapMallocLimited

void ICall_heapMgrGetMetrics(uint32_t * pBlkMax, uint32_t * pBlkCnt, uint32_t * pBlkFree, uint32_t * pMemAlo, uint32_t * pMemMax,
                             uint32_t * pMemUB);
#ifdef HEAPMGR_METRICS
#define HEAPMGR_GETMETRICS ICall_heapMgrGetMetrics
#endif

#define HEAPMGR_LOCK()                                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        ICall_heapCSState = ICall_enterCSImpl();                                                                                   \
    } while (0)
#define HEAPMGR_UNLOCK()                                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        ICall_leaveCSImpl(ICall_heapCSState);                                                                                      \
    } while (0)
#define HEAPMGR_IMPL_INIT()
/* Note that a static variable can be used to contain critical section
 * state since heapmgr.h template ensures that there is no nested
 * lock call. */

#if defined(HEAPMGR_CONFIG) && ((HEAPMGR_CONFIG == 0) || (HEAPMGR_CONFIG == 0x80))
#include <rtos_heaposal.h>
#elif defined(HEAPMGR_CONFIG) && ((HEAPMGR_CONFIG == 1) || (HEAPMGR_CONFIG == 0x81))
#include <rtos_heapmem.h>
#elif defined(HEAPMGR_CONFIG) && ((HEAPMGR_CONFIG == 2) || (HEAPMGR_CONFIG == 0x82))
#include <rtos_heaptrack.h>
#elif defined(FREERTOS)
#include "TI_heap_wrapper.h"
#else
static ICall_CSState ICall_heapCSState;
#include <rtos_heaposal.h>
#endif

/**
 * @internal Searches for a task entry within @ref ICall_tasks.
 * @param taskhandle  OS task handle
 * @return Pointer to task entry when found, or NULL.
 */
static ICall_TaskEntry * ICall_searchTask(TaskHandle_t taskhandle)
{
    size_t i;
    ICall_CSState key;

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_TASKS; i++)
    {
        if (!ICall_tasks[i].task)
        {
            /* Empty slot */
            break;
        }
        if ((TaskHandle_t) taskhandle == (TaskHandle_t) ICall_tasks[i].task)
        {
            ICall_leaveCSImpl(key);
            return &ICall_tasks[i];
        }
    }
    ICall_leaveCSImpl(key);
    return NULL;
}

/**
 * @internal Searches for a task entry within @ref ICall_tasks or
 *           build an entry if the entry table is empty.
 * @param  taskhandle  OS task handle
 * @return Pointer to task entry when found, or NULL.
 */

static ICall_TaskEntry * ICall_newTask(TaskHandle_t taskhandle)
{
    size_t i;
    ICall_CSState key;

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_TASKS; i++)
    {
        if (!ICall_tasks[i].task)
        {
            /* Empty slot */
            ICall_TaskEntry * taskentry = &ICall_tasks[i];
            taskentry->task             = taskhandle;
            taskentry->queue            = NULL;

#ifdef FREERTOS
            taskentry->syncHandle = xQueueCreate(20, sizeof(uint32_t));
#endif

            if (taskentry->syncHandle == 0)
            {
                /* abort */
                ICALL_HOOK_ABORT_FUNC();
            }

            ICall_leaveCSImpl(key);
            return taskentry;
        }
        if (taskhandle == (TaskHandle_t) ICall_tasks[i].task)
        {
            ICall_leaveCSImpl(key);
            return &ICall_tasks[i];
        }
    }
    ICall_leaveCSImpl(key);
    return NULL;
}

/* See header file for comments. */
ICall_EntityID ICall_searchServiceEntity(ICall_ServiceEnum service)
{
    size_t i;
    ICall_CSState key;

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Empty slot */
            break;
        }
        if (service == ICall_entities[i].service)
        {
            ICall_leaveCSImpl(key);
            return (ICall_EntityID) i;
        }
    }
    ICall_leaveCSImpl(key);
    return ICALL_INVALID_ENTITY_ID;
}

/**
 * @internal Searches for a service entity entry.
 * @param service  service id
 * @return Pointer to entity entry of the service or
 *         NULL when none found.
 */
static ICall_entityEntry * ICall_searchService(ICall_ServiceEnum service)
{
    ICall_EntityID entity = ICall_searchServiceEntity(service);
    if (entity == ICALL_INVALID_ENTITY_ID)
    {
        return NULL;
    }
    return &ICall_entities[entity];
}

/* Dispatcher implementation. See ICall_dispatcher declaration
 * for comment. */
static ICall_Errno ICall_dispatch(ICall_FuncArgsHdr * args)
{
    ICall_entityEntry * entity;

    entity = ICall_searchService(args->service);
    if (!entity)
    {
        return ICALL_ERRNO_INVALID_SERVICE;
    }
    if (!entity->fn)
    {
        return ICALL_ERRNO_INVALID_FUNCTION;
    }

    return entity->fn(args);
}

/* See header file for comments */
ICall_Dispatcher ICall_dispatcher = ICall_dispatch;

/* Static instance of ICall_RemoteTaskArg to pass to
 * remote task entry function.
 * See header file for comments */
static const ICall_RemoteTaskArg ICall_taskEntryFuncs = { ICall_dispatch, ICall_enterCSImpl, ICall_leaveCSImpl };

/**
 * @internal Thread entry function wrapper that complies with
 *           OS.
 * @param arg0  actual entry function
 * @param arg1  ignored
 */
TaskHandle_t RemoteTask = NULL;

// pthread_t RemoteTask;

struct argsForPosixTaskStart
{
    void * arg0;
    void * arg1;
};
struct argsForPosixTaskStart POSIX_args;

typedef void (*TaskFunction_t)(void *);

static void ICall_taskEntry(void * arg)

{
    void * arg0 = ((struct argsForPosixTaskStart *) (arg))->arg0;
    void * arg1 = ((struct argsForPosixTaskStart *) (arg))->arg1;

    ICall_CSState key;
    key = ICall_enterCSImpl();
    if (ICall_newTask(ICall_taskSelf()) == NULL)
    {
        /* abort */
        ICALL_HOOK_ABORT_FUNC();
    }
    ICall_leaveCSImpl(key);

    /* Attempt to yield prior to running task */
    taskYIELD();

    ICall_RemoteTaskEntry entryfn = (ICall_RemoteTaskEntry) arg0;

    entryfn(&ICall_taskEntryFuncs, (void *) arg1);

    // return NULL;
}

#ifndef ICALL_JT
/* forward reference */
static void ICall_initPrim(void);
#endif /* ICALL_JT */

/* See header file for comments. */
void ICall_init(void)
{
    size_t i;

    for (i = 0; i < ICALL_MAX_NUM_TASKS; i++)
    {
        ICall_tasks[i].task  = NULL;
        ICall_tasks[i].queue = NULL;
    }
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        ICall_entities[i].service = ICALL_SERVICE_CLASS_INVALID_ENTRY;
    }

#ifndef ICALL_JT
    /* Initialize primitive service */
    ICall_initPrim();
#else
    /* Initialize heap */
#ifndef FREERTOS
    ICall_heapInit();
#endif // FREERTOS
#endif
}

/* See header file for comments */
void ICall_createRemoteTasksAtRuntime(ICall_RemoteTask_t * remoteTaskTable, uint8_t nbElems)
{
    size_t i;
    /* ICALL_Task_disable is a cheap locking mechanism to lock tasks
     * which may attempt to access the service call dispatcher
     * till all services are registered.
     */
    UBaseType_t OriginalParam;
    ICALL_Task_disable(&OriginalParam);
    for (i = 0; i < nbElems; i++)
    {
#ifdef FREERTOS
        BaseType_t xReturned;

        /* Pass the args via external sturct (POSIX use only single arg) */
        POSIX_args.arg0 = (void *) remoteTaskTable[i].startupEntry;
        POSIX_args.arg1 = (void *) remoteTaskTable[i].ICall_imgInitParam;

        xReturned = xTaskCreate(ICall_taskEntry,                                        /* Function that implements the task. */
                                "x",                                                    /* Text name for the task. */
                                remoteTaskTable[i].imgTaskStackSize / sizeof(uint32_t), /* Stack size in words, not bytes. */
                                (void *) &POSIX_args,                                   /* Parameter passed into the task. */
                                remoteTaskTable[i].imgTaskPriority,                     /* Priority at which the task is created. */
                                &RemoteTask); /* Used to pass out the created task's handle. */

        if (xReturned == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
        {
            /* Creation of FreeRTOS task failed */
            while (1)
                ;
        }
    }
    ICALL_Task_restore(&OriginalParam);
#endif
}

/* See header file for comments */
void ICall_createRemoteTasks(void)
{
    size_t i;
    ICall_RemoteTask_t remoteTaskTable[ICALL_REMOTE_THREAD_COUNT];

    for (i = 0; i < ICALL_REMOTE_THREAD_COUNT; i++)
    {
        remoteTaskTable[i].imgTaskPriority    = ICall_threadPriorities[i];
        remoteTaskTable[i].imgTaskStackSize   = ICall_threadStackSizes[i];
        remoteTaskTable[i].startupEntry       = icall_threadEntries[i];
        remoteTaskTable[i].ICall_imgInitParam = (void *) ICall_getInitParams(i);
    }
    ICall_createRemoteTasksAtRuntime(remoteTaskTable, ICALL_REMOTE_THREAD_COUNT);
}

#ifdef FREERTOS
void ICALL_Task_disable(UBaseType_t * OriginalParam)

#else
    void ICALL_Task_disable(struct sched_param * OriginalParam)
#endif
{

#ifdef FREERTOS
    TaskStatus_t pxTaskStatus;
    vTaskGetInfo(ICall_taskSelf(), &pxTaskStatus, pdFALSE, eInvalid);
    *OriginalParam = pxTaskStatus.uxCurrentPriority;

    vTaskPrioritySet(ICall_taskSelf(), configMAX_PRIORITIES - 1);
#endif
}
#ifdef FREERTOS
void ICALL_Task_restore(UBaseType_t * OriginalParam)
#else
    void ICALL_Task_restore(struct sched_param * OriginalParam)
#endif
{

#ifdef FREERTOS
    vTaskPrioritySet(ICall_taskSelf(), *OriginalParam);

#else
        pthread_t pthreadID = pthread_self();
        pthread_setschedparam(pthreadID, 0, OriginalParam);
#endif
}

/* See header file for comments */
ICall_TaskHandle ICall_getRemoteTaskHandle(uint8 index)
{
    TaskHandle_t * task = NULL;

    UBaseType_t OriginalParam;
    ICALL_Task_disable(&OriginalParam);

    if (index < ICALL_MAX_NUM_TASKS)
    {
        task = &ICall_tasks[index].task;
    }

    ICALL_Task_restore(&OriginalParam);

    return ((ICall_TaskHandle) task);
}

/* Primitive service implementation follows */

#ifndef ICALL_JT
/**
 * @internal Enrolls a service
 * @param args arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when service id is already
 *         registered by another entity.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when maximum number of services
 *         are already registered.
 */
static ICall_Errno ICall_primEnroll(ICall_EnrollServiceArgs * args)
{
    size_t i;
    ICall_TaskEntry * taskentry = ICall_newTask(Task_self());
    ICall_CSState key;

    /* Note that certain service does not handle a message
     * and hence, taskentry might be NULL.
     */
    if (taskentry == NULL)
    {
        return ICALL_ERRNO_INVALID_PARAMETER;
    }

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Use this entry */
            ICall_entities[i].service = args->service;
            ICall_entities[i].task    = taskentry;
            ICall_entities[i].fn      = args->fn;
            args->entity              = (ICall_EntityID) i;
            args->msgSyncHdl          = taskentry->syncHandle;
            ICall_leaveCSImpl(key);
            return ICALL_ERRNO_SUCCESS;
        }
        else if (args->service == ICall_entities[i].service)
        {
            /* Duplicate service enrollment */
            ICall_leaveCSImpl(key);
            return ICALL_ERRNO_INVALID_PARAMETER;
        }
    }
    /* abort */
    ICALL_HOOK_ABORT_FUNC();
    ICall_leaveCSImpl(key);
    return ICALL_ERRNO_NO_RESOURCE;
}

/**
 * @internal Registers an application
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 */
static ICall_Errno ICall_primRegisterApp(ICall_RegisterAppArgs * args)
{
    size_t i;
    ICall_TaskEntry * taskentry = ICall_newTask(Task_self());
    ICall_CSState key;

    if (!taskentry)
    {
        /* abort */
        ICALL_HOOK_ABORT_FUNC();
        return ICALL_ERRNO_NO_RESOURCE;
    }

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Use this entry */
            ICall_entities[i].service = ICALL_SERVICE_CLASS_APPLICATION;
            ICall_entities[i].task    = taskentry;
            ICall_entities[i].fn      = NULL;
            args->entity              = (ICall_EntityID) i;
            args->msgSyncHdl          = taskentry->syncHandle;
            ICall_leaveCSImpl(key);
            return ICALL_ERRNO_SUCCESS;
        }
    }
    /* abort */
    ICALL_HOOK_ABORT_FUNC();
    ICall_leaveCSImpl(key);
    return ICALL_ERRNO_NO_RESOURCE;
}

/**
 * @internal Allocates memory block for a message.
 * @param args   arguments
 */
static ICall_Errno ICall_primAllocMsg(ICall_AllocArgs * args)
{
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) ICall_heapMalloc(sizeof(ICall_MsgHdr) + args->size);

    if (!hdr)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }
    hdr->len     = args->size;
    hdr->next    = NULL;
    hdr->dest_id = ICALL_UNDEF_DEST_ID;
    args->ptr    = (void *) (hdr + 1);
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Frees the memory block allocated for a message.
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS
 */
static ICall_Errno ICall_primFreeMsg(ICall_FreeArgs * args)
{
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) args->ptr - 1;
    ICall_heapFree(hdr);
    return ICALL_ERRNO_SUCCESS;
}

/**
 * Allocates a memory block.
 * Note that this function is for use by ICall implementation.
 *
 * @param size   size in bytes
 * @return pointer to the allocated memory block or NULL
 */
void * ICall_mallocImpl(uint_fast16_t size)
{
    return ICall_heapMalloc(size);
}

/**
 * Frees a memory block.
 * Note that this function is for use by ICall implementation.
 *
 * @param ptr   pointer to the memory block
 */
void ICall_freeImpl(void * ptr)
{
    ICall_heapFree(ptr);
}

/**
 * @internal Allocates a memory block
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when memory block cannot
 *         be allocated.
 */
static ICall_Errno ICall_primMalloc(ICall_AllocArgs * args)
{
    args->ptr = ICall_heapMalloc(args->size);
    if (args->ptr == NULL)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Frees a memory block
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS
 */
static ICall_Errno ICall_primFree(ICall_FreeArgs * args)
{
    ICall_heapFree(args->ptr);
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_JT */

/**
 * @internal Queues a message to a message queue.
 * @param q_ptr    message queue
 * @param msg_ptr  message pointer
 */
static void ICall_msgEnqueue(ICall_MsgQueue * q_ptr, void * msg_ptr)
{
    void * list;
    ICall_CSState key;

    // Hold off interrupts
    key = ICall_enterCSImpl();

    ICALL_MSG_NEXT(msg_ptr) = NULL;
    // If first message in queue
    if (*q_ptr == NULL)
    {
        *q_ptr = msg_ptr;
    }
    else
    {
        // Find end of queue
        for (list = *q_ptr; ICALL_MSG_NEXT(list) != NULL; list = ICALL_MSG_NEXT(list))
            ;

        // Add message to end of queue
        ICALL_MSG_NEXT(list) = msg_ptr;
    }

    // Re-enable interrupts
    ICall_leaveCSImpl(key);
}

/**
 * @internal Dequeues a message from a message queue
 * @param q_ptr  message queue pointer
 * @return Dequeued message pointer or NULL if none.
 */
static void * ICall_msgDequeue(ICall_MsgQueue * q_ptr)
{
    void * msg_ptr = NULL;
    ICall_CSState key;

    // Hold off interrupts
    key = ICall_enterCSImpl();

    if (*q_ptr != NULL)
    {
        // Dequeue message
        msg_ptr                    = *q_ptr;
        *q_ptr                     = ICALL_MSG_NEXT(msg_ptr);
        ICALL_MSG_NEXT(msg_ptr)    = NULL;
        ICALL_MSG_DEST_ID(msg_ptr) = ICALL_UNDEF_DEST_ID;
    }

    // Re-enable interrupts
    ICall_leaveCSImpl(key);

    return msg_ptr;
}

/**
 * @internal Prepends a list of messages to a message queue
 * @param q_ptr  message queue pointer
 * @param head   message list to prepend
 */
static void ICall_msgPrepend(ICall_MsgQueue * q_ptr, ICall_MsgQueue head)
{
    void * msg_ptr = NULL;
    ICall_CSState key;

    // Hold off interrupts
    key = ICall_enterCSImpl();

    if (head != NULL)
    {
        /* Find the end of the queue */
        msg_ptr = head;
        while (ICALL_MSG_NEXT(msg_ptr) != NULL)
        {
            msg_ptr = ICALL_MSG_NEXT(msg_ptr);
        }
        ICALL_MSG_NEXT(msg_ptr) = *q_ptr;
        *q_ptr                  = head;
    }

    // Re-enable interrupts
    ICall_leaveCSImpl(key);
}

#ifndef ICALL_JT
/**
 * @internal Sends a message to an entity.
 * @param args    arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when either src
 *              or dest is not a valid entity id or when
 *              dest is an entity id of an entity that does
 *              not receive a message
 *              (e.g., ICall primitive service entity).
 */
static ICall_Errno ICall_primSend(ICall_SendArgs * args)
{
    ICall_CSState key;
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) args->msg - 1;

    if (args->dest.entityId >= ICALL_MAX_NUM_ENTITIES || args->src >= ICALL_MAX_NUM_ENTITIES)
    {
        return ICALL_ERRNO_INVALID_PARAMETER;
    }
    key = ICall_enterCSImpl();
    if (!ICall_entities[args->dest.entityId].task)
    {
        ICall_leaveCSImpl(key);
        return ICALL_ERRNO_INVALID_PARAMETER;
    }

    ICall_leaveCSImpl(key);
    /* Note that once the entry is valid,
     * the value does not change and hence it is OK
     * to leave the critical section.
     */

    hdr->srcentity = args->src;
    hdr->dstentity = args->dest.entityId;
    hdr->format    = args->format;
    ICall_msgEnqueue(&ICall_entities[args->dest.entityId].task->queue, args->msg);
    ICALL_SYNC_HANDLE_POST(ICall_entities[args->dest.entityId].task->syncHandle);

    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Retrieves a message, queued to receive queue of the calling thread.
 *
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS when a message was successfully
 *         retrieved.<br>
 *         @ref ICALL_ERRNO_NOMSG when no message was queued to
 *         the receive queue at the moment.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when the calling thread
 *         does not have a received queue associated with it.
 *         This happens when neither ICall_enrollService() nor
 *         ICall_registerApp() was ever called from the calling
 *         thread.
 */
static ICall_Errno ICall_primFetchMsg(ICall_FetchMsgArgs * args)
{
    Task_Handle taskhandle      = Task_self();
    ICall_TaskEntry * taskentry = ICall_searchTask(taskhandle);
    ICall_MsgHdr * hdr;

    if (!taskentry)
    {
        return ICALL_ERRNO_UNKNOWN_THREAD;
    }
    /* Successful */
    args->msg = ICall_msgDequeue(&taskentry->queue);

    if (args->msg == NULL)
    {
        return ICALL_ERRNO_NOMSG;
    }
    hdr                = (ICall_MsgHdr *) args->msg - 1;
    args->src.entityId = hdr->srcentity;
    args->dest         = hdr->dstentity;
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_JT */

/**
 * @internal
 * Transforms and entityId into a serviceId.
 * @param entityId   entity id
 * @param servId     pointer to a variable to store
 *                   the resultant service id
 * @return @ICALL_ERRNO_SUCCESS if the transformation was successful.
 *         @ICALL_ERRNO_INVALID_SERVICE if no matching service
 *         is found for the entity id.
 */
static ICall_Errno ICall_primEntityId2ServiceId(ICall_EntityID entityId, ICall_ServiceEnum * servId)
{
    if (entityId >= ICALL_MAX_NUM_ENTITIES || ICall_entities[entityId].service == ICALL_SERVICE_CLASS_INVALID_ENTRY ||
        ICall_entities[entityId].service == ICALL_SERVICE_CLASS_APPLICATION)
    {
        return ICALL_ERRNO_INVALID_SERVICE;
    }
    *servId = ICall_entities[entityId].service;
    return ICALL_ERRNO_SUCCESS;
}

#ifndef ICALL_JT
/**
 * @internal Transforms and entityId into a serviceId.
 * @param args  arguments
 * @return return values corresponding to those of ICall_entityId2ServiceId()
 */
static ICall_Errno ICall_primE2S(ICall_EntityId2ServiceIdArgs * args)
{
    return ICall_primEntityId2ServiceId(args->entityId, &args->servId);
}

/**
 * @internal Sends a message to a registered server.
 * @param args   arguments corresponding to those of ICall_sendServiceMsg().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_SERVICE when the 'dest'
 *         is unregistered service.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the 'src'
 *         is not a valid entity id or when 'dest' is
 *         is a service that does not receive a message
 *         (such as ICall primitive service).
 */
static ICall_Errno ICall_primSendServiceMsg(ICall_SendArgs * args)
{
    ICall_EntityID dstentity = ICall_searchServiceEntity(args->dest.servId);

    if (dstentity == ICALL_INVALID_ENTITY_ID)
    {
        return ICALL_ERRNO_INVALID_SERVICE;
    }
    args->dest.entityId = dstentity;
    return ICall_primSend(args);
}

/**
 * @internal Retrieves a message received at the message queue
 * associated with the calling thread.
 *
 * Note that this function should be used by an application
 * which does not expect any message from non-server entity.
 *
 * @param args   arguments corresponding to those of ICall_fetchServiceMsg()
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 *         and a message was retrieved.<br>
 *         @ref ICALL_ERRNO_NOMSG when there is no queued message
 *         at the moment.<br>
 *         @ref ICALL_ERRNO_CORRUPT_MSG when a message queued in
 *         front of the thread's receive queue was not sent by
 *         a server. Note that in this case, the message is
 *         not retrieved but thrown away.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when this function is
 *         called from a thread which has not registered
 *         an entity, either through ICall_enrollService()
 *         or through ICall_registerApp().
 */
static ICall_Errno ICall_primFetchServiceMsg(ICall_FetchMsgArgs * args)
{
    ICall_ServiceEnum servId;
    ICall_Errno errno = ICall_primFetchMsg(args);
    if (errno == ICALL_ERRNO_SUCCESS)
    {
        if (ICall_primEntityId2ServiceId(args->src.entityId, &servId) != ICALL_ERRNO_SUCCESS)
        {
            /* Source entity ID cannot be translated to service id */
            ICall_freeMsg(args->msg);
            return ICALL_ERRNO_CORRUPT_MSG;
        }
        args->src.servId = servId;

#ifdef ICALL_EVENTS
        /*
         * Because Events are binary flags, the task's queue must be checked for
         * any remaining messages.  If there are the ICall event flag must be
         * re-posted due to it being cleared on the last pend.
         */
        ICall_primRepostSync();
#endif // ICALL_EVENTS
    }
    return errno;
}
#endif /* ICALL_JT */
/**
 * @internal
 * Converts milliseconds to number of ticks.
 * @param msecs milliseconds
 * @param ticks pointer to a variable to store the resultant number of ticks
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when conversion failed
 *         as the input goes out of range for the output data type.
 */
static ICall_Errno ICall_msecs2Ticks(uint_fast32_t msecs, uint32_t * ticks)
{
    uint_fast64_t intermediate = msecs;

    /*convert to microSec*/
    intermediate *= 1000;
    /*divide with the ticks perios*/
    intermediate /= ICall_getTickPeriod();
    if (intermediate >= ((uint_fast64_t) 1 << (sizeof(uint32_t) * 8 - 1)))
    {
        /* Out of range.
         * Note that we use only half of the range so that client can
         * determine whether the time has passed or time has yet to come.
         */
        return ICALL_ERRNO_INVALID_PARAMETER;
    }
    *ticks = (uint32_t) intermediate;
    return ICALL_ERRNO_SUCCESS;
}

#ifndef ICALL_JT
/**
 * @internal
 * Waits for a signal to the synchronization object associated with the calling
 * thread.
 *
 * Note that the synchronization object associated with a thread is signaled
 * when a message is queued to the message receive queue of the thread
 * or when ICall_signal() function is called onto the synchronization object.
 *
 * @param args  arguments corresponding to those of ICall_wait().
 * @return @ref ICALL_ERRNO_SUCCESS when the synchronization object is
 *         signaled.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the synchronization object being signaled.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the milliseconds
 *         is greater than the value of ICall_getMaxMSecs().
 */
static ICall_Errno ICall_primWait(ICall_WaitArgs * args)
{
    Task_Handle taskhandle      = Task_self();
    ICall_TaskEntry * taskentry = ICall_searchTask(taskhandle);
    uint32_t timeout;

    {
        BIOS_ThreadType threadtype = BIOS_getThreadType();

        if (threadtype == BIOS_ThreadType_Hwi || threadtype == BIOS_ThreadType_Swi)
        {
            /* Blocking call is not allowed from Hwi or Swi.
             * Note that though theoretically, Swi or lower priority Hwi may block
             * on an event to be generated by a higher priority Hwi, it is not a
             * safe practice and hence it is disabled.
             */
            return ICALL_ERRNO_UNKNOWN_THREAD;
        }
    }

    if (!taskentry)
    {
        return ICALL_ERRNO_UNKNOWN_THREAD;
    }
    /* Successful */
    if (args->milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        /* Convert milliseconds to number of ticks */
        ICall_Errno errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return errno;
        }
    }

    if (ICALL_SYNC_HANDLE_PEND(taskentry->syncHandle, timeout))
    {
        return ICALL_ERRNO_SUCCESS;
    }

    return ICALL_ERRNO_TIMEOUT;
}

/**
 * @internal signals a synchronziation object.
 * @param args  arguments corresponding to those of ICall_signal()
 * @return return value corresponding to those of ICall_signal()
 */
static ICall_Errno ICall_primSignal(ICall_SignalArgs * args)
{
    ICALL_SYNC_HANDLE_POST(args->syncHandle);
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal aborts program
 * @param args  arguments corresponding to those of ICall_abort()
 * @return return value corresponding to those of ICall_abort()
 */
static ICall_Errno ICall_primAbort(ICall_FuncArgsHdr * args)
{
    ICALL_HOOK_ABORT_FUNC();
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Enables an interrupt.
 * @param args arguments corresponding to those of ICall_enableint()
 * @return return values corresponding to those of ICall_enableint()
 */
static ICall_Errno ICall_primEnableint(ICall_intNumArgs * args)
{
    Hwi_enableinterrupt(args->intnum);
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Disables an interrupt.
 * @param args arguments corresponding to those of ICall_disableint()
 * @return return values corresponding to those of ICall_disableint()
 */
static ICall_Errno ICall_primDisableint(ICall_intNumArgs * args)
{
    Hwi_disableinterrupt(args->intnum);
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Enables master interrupt and context switching.
 * @param args arguments corresponding to those of ICall_enableMint()
 * @return return values corresponding to those of ICall_enableMint()
 */
static ICall_Errno ICall_primEnableMint(ICall_FuncArgsHdr * args)
{
    Hwi_enable();
    Swi_enable();
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Disables master interrupt and context switching.
 * @param args arguments corresponding to those of ICall_disableMint()
 * @return return values corresponding to those of ICall_disableMint()
 */
static ICall_Errno ICall_primDisableMint(ICall_FuncArgsHdr * args)
{
    Swi_disable();
    Hwi_disable();
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal registers an interrupt service routine
 * @param args  arguments corresponding to those of ICall_registerISR()
 * @return return values corresponding to those of ICall_registerISR()
 */
static ICall_Errno ICall_primRegisterISR(ICall_RegisterISRArgs * args)
{
    Hwi_Params hwiParams;

    Hwi_Params_init(&hwiParams);
    hwiParams.priority = 0xE0; // default all registered ints to lowest priority

    if (Hwi_create(args->intnum, (void (*)((void *) )) args->isrfunc, &hwiParams, NULL) == NULL)
    {
        ICALL_HOOK_ABORT_FUNC();
        return ICALL_ERRNO_NO_RESOURCE;
    }
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal registers an interrupt service routine
 * @param args  arguments corresponding to those of ICall_registerISR_Ext()
 * @return return values corresponding to those of ICall_registerISR_ext()
 */
static ICall_Errno ICall_primRegisterISR_Ext(ICall_RegisterISRArgs_Ext * args)
{
    Hwi_Params hwiParams;

    Hwi_Params_init(&hwiParams);
    hwiParams.priority = args->intPriority;

    if (Hwi_create(args->intnum, (void (*)((void *) )) args->isrfunc, &hwiParams, NULL) == NULL)
    {
        ICALL_HOOK_ABORT_FUNC();
        return ICALL_ERRNO_NO_RESOURCE;
    }
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Gets tick counter value
 * @param args  arguments corresponding to those of ICall_getTicks()
 * @return return values corresponding to those of ICall_getTicks()
 */
static ICall_Errno ICall_primGetTicks(ICall_Getuint32_tArgs * args)
{
    args->value = Clock_getTicks();
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_JT */

/**
 * @internal
 * Clock event handler function.
 * This function is used to implement the wakeup scheduler.
 *
 * @param arg  an @ref ICall_ScheduleEntry
 */

static void ICall_clockFunc(uintptr_t arg)
{
    ICall_ScheduleEntry * entry = (ICall_ScheduleEntry *) arg;

    entry->cback(entry->arg);
}

#ifndef ICALL_JT
/**
 * @internal
 * Set up or restart a timer.
 *
 * @param args arguments corresponding to those of ICall_setTimer()
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER if timer designated by the
 *              timer ID value was not set up before.
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 *         Check ICall heap size and OS heap size if this happens.
 */
static ICall_Errno ICall_primSetTimer(ICall_SetTimerArgs * args)
{
    ICall_ScheduleEntry * entry;

    if (args->timerid == ICALL_INVALID_TIMER_ID)
    {
        Clock_Params params;

        /* Create a new timer */
        entry = ICall_heapMalloc(sizeof(ICall_ScheduleEntry));
        if (entry == NULL)
        {
            /* allocation failed */
            return ICALL_ERRNO_NO_RESOURCE;
        }
        Clock_Params_init(&params);
        params.startFlag = FALSE;
        params.period    = 0;
        params.arg       = ((void *) ) entry;
        entry->clock     = Clock_create(ICall_clockFunc, args->timeout, &params, NULL);
        if (!entry->clock)
        {
            /* abort */
            ICall_abort();
            ICall_heapFree(entry);
            return ICALL_ERRNO_NO_RESOURCE;
        }
        entry->cback  = args->cback;
        entry->arg    = args->arg;
        args->timerid = (ICall_TimerID) entry;
    }
    else
    {
        ICall_CSState key;

        entry = (ICall_ScheduleEntry *) args->timerid;

        /* Critical section is entered to disable interrupts that might cause call
         * to callback due to race condition */
        key = ICall_enterCriticalSection();
        Clock_stop(entry->clock);
        entry->arg = args->arg;
        ICall_leaveCriticalSection(key);
    }

    Clock_setTimeout(entry->clock, args->timeout);
    Clock_start(entry->clock);

    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Set up or restart a timer.
 *
 * @param args arguments corresponding to those of ICall_setTimerMSecs()
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when msecs is greater than
 *              maximum value supported.
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 *         Check ICall heap size and OS heap size if this happens.
 */
static ICall_Errno ICall_primSetTimerMSecs(ICall_SetTimerArgs * args)
{
    uint32_t ticks;
    /* Convert to tick time */
    ICall_Errno errno = ICall_msecs2Ticks(args->timeout, &ticks);

    if (errno != ICALL_ERRNO_SUCCESS)
    {
        return errno;
    }
    args->timeout = ticks;
    return ICall_primSetTimer(args);
}

/**
 * @internal
 * Stops a timer.
 *
 * @param args arguments corresponding to those of ICall_stopTimer()
 *
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER
 *              if id is @ref ICALL_INVALID_TIMER_ID.
 */
static ICall_Errno ICall_primStopTimer(ICall_StopTimerArgs * args)
{
    ICall_ScheduleEntry * entry = (ICall_ScheduleEntry *) args->timerid;

    if (args->timerid == ICALL_INVALID_TIMER_ID)
    {
        return ICALL_ERRNO_INVALID_PARAMETER;
    }

    Clock_stop(entry->clock);
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Gets tick period
 * @param args  arguments corresponding to those of ICall_getTickPeriod()
 * @return return values corresponding to those of ICall_getTickPeriod()
 */
static ICall_Errno ICall_primGetTickPeriod(ICall_Getuint32_tArgs * args)
{
    args->value = Clock_tickPeriod;
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Gets maximum period supported
 * @param args  arguments corresponding to those of ICall_getMaxMSecs()
 * @return return values corresponding to those of ICall_getMaxMSecs()
 */
static ICall_Errno ICall_primGetMaxMSecs(ICall_Getuint32_tArgs * args)
{
    uint_fast64_t tmp = ((uint_fast64_t) 0x7ffffffful) * Clock_tickPeriod;
    tmp /= 1000;
    if (tmp >= 0x80000000ul)
    {
        tmp = 0x7ffffffful;
    }
    args->value = (uint_least32_t) tmp;
    return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Waits for a message that matches comparison
 *
 * @param args  arguments corresponding to those of ICall_waitMatch().
 * @return @ref ICALL_ERRNO_SUCCESS when the synchronization object is
 *         signaled.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the semaphore being signaled.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the milliseconds
 *         is greater than the value of ICall_getMaxMSecs().
 */
static ICall_Errno ICall_primWaitMatch(ICall_WaitMatchArgs * args)
{
    Task_Handle taskhandle      = Task_self();
    ICall_TaskEntry * taskentry = ICall_searchTask(taskhandle);
    ICall_MsgQueue prependQueue = NULL;
#ifndef ICALL_EVENTS
    uint_fast16_t consumedCount = 0;
#endif
    uint32_t timeout;
    uint_fast32_t timeoutStamp;
    ICall_Errno errno;

    {
        BIOS_ThreadType threadtype = BIOS_getThreadType();

        if (threadtype == BIOS_ThreadType_Hwi || threadtype == BIOS_ThreadType_Swi)
        {
            /* Blocking call is not allowed from Hwi or Swi.
             * Note that though theoretically, Swi or lower priority Hwi may block
             * on an event to be generated by a higher priority Hwi, it is not a
             * safe practice and hence it is disabled.
             */
            return ICALL_ERRNO_UNKNOWN_THREAD;
        }
    }

    if (!taskentry)
    {
        return ICALL_ERRNO_UNKNOWN_THREAD;
    }
    /* Successful */
    if (args->milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        /* Convert milliseconds to number of ticks */
        errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return errno;
        }
    }

    errno        = ICALL_ERRNO_TIMEOUT;
    timeoutStamp = Clock_getTicks() + timeout;
    while (ICALL_SYNC_HANDLE_PEND(taskentry->syncHandle, timeout))
    {
        ICall_FetchMsgArgs fetchArgs;
        ICall_ServiceEnum servId;
        errno = ICall_primFetchMsg(&fetchArgs);
        if (errno == ICALL_ERRNO_SUCCESS)
        {
            if (ICall_primEntityId2ServiceId(fetchArgs.src.entityId, &servId) == ICALL_ERRNO_SUCCESS)
            {
                if (args->matchFn(servId, fetchArgs.dest, fetchArgs.msg))
                {
                    /* Matching message found*/
                    args->servId = servId;
                    args->dest   = fetchArgs.dest;
                    args->msg    = fetchArgs.msg;
                    errno        = ICALL_ERRNO_SUCCESS;
                    break;
                }
            }
            /* Message was received but it wasn't expected one.
             * Add to the prepend queue */
            ICall_msgEnqueue(&prependQueue, fetchArgs.msg);
#ifdef ICALL_EVENTS
            /* Event are binary semaphore, so if several messsages are posted while
             * we are processing one, it's possible that some of them are 'missed' and
             * not processed. Sending a event to ourself force this loop to run until
             * all the messages in the queue are processed.
             */
            ICALL_SYNC_HANDLE_POST(taskentry->syncHandle);
#endif
        }

        /* Prepare for timeout exit */
        errno = ICALL_ERRNO_TIMEOUT;

#ifndef ICALL_EVENTS
        /* Keep the decremented semaphore count */
        consumedCount++;
#endif /* ICALL_EVENTS */
        if (timeout != BIOS_WAIT_FOREVER && timeout != BIOS_NO_WAIT)
        {
            /* Readjust timeout */
            uint32_t newTimeout = timeoutStamp - Clock_getTicks();
            if (newTimeout == 0 || newTimeout > timeout)
            {
                break;
            }
            timeout = newTimeout;
        }
    }

#ifdef ICALL_EVENTS
    /*
     * Because Events are binary semaphores, the task's queue must be checked for
     * any remaining messages.  If there are, the ICall event flag must be
     * re-posted due to it being cleared on the last pend.
     */
    ICall_primRepostSync();
#endif // ICALL_EVENTS

    /* Prepend retrieved irrelevant messages */
    ICall_msgPrepend(&taskentry->queue, prependQueue);
#ifndef ICALL_EVENTS
    /* Re-increment the consumed semaphores */
    for (; consumedCount > 0; consumedCount--)
    {
        Semaphore_post(taskentry->syncHandle);
    }
#endif /* ICALL_EVENTS */
    return errno;
}

/**
 * @internal
 * Retrieves an entity ID of an entity associated with the calling thread.
 *
 * @param args  arguments corresponding to those of ICall_getEntityId().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when no entity was associated
 *         with the calling thread.
 */
static ICall_Errno ICall_primGetEntityId(ICall_GetEntityIdArgs * args)
{
    Task_Handle taskhandle = Task_self();
    ICall_CSState key;
    size_t i;

    {
        BIOS_ThreadType threadtype = BIOS_getThreadType();

        if (threadtype == BIOS_ThreadType_Hwi || threadtype == BIOS_ThreadType_Swi)
        {
            return ICALL_ERRNO_UNKNOWN_THREAD;
        }
    }

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Not found */
            break;
        }
        if (ICall_entities[i].task->task == taskhandle)
        {
            /* Found */
            args->entity = i;
            ICall_leaveCSImpl(key);
            return ICALL_ERRNO_SUCCESS;
        }
    }
    ICall_leaveCSImpl(key);
    return ICALL_ERRNO_UNKNOWN_THREAD;
}

/**
 * @internal
 * Checks whether the calling thread provides the designated service.
 *
 * @param args  arguments corresponding to those of ICall_threadServes().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when the calling thread is
 *         unrecognized.
 *         @ref ICALL_ERRNO_INVALID_SERVICE if the service id is not enrolled
 *         by any thread.
 */
static ICall_Errno ICall_primThreadServes(ICall_ThreadServesArgs * args)
{
    Task_Handle taskhandle;
    ICall_CSState key;
    size_t i;

    {
        BIOS_ThreadType threadtype = BIOS_getThreadType();

        if (threadtype == BIOS_ThreadType_Hwi || threadtype == BIOS_ThreadType_Swi)
        {
            return ICALL_ERRNO_UNKNOWN_THREAD;
        }
    }

    taskhandle = Task_self();

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Not found */
            break;
        }
        else if (ICall_entities[i].service == args->servId)
        {
            args->result = (uint_fast8_t) (ICall_entities[i].task->task == taskhandle);
            ICall_leaveCSImpl(key);
            return ICALL_ERRNO_SUCCESS;
        }
    }
    ICall_leaveCSImpl(key);
    return ICALL_ERRNO_INVALID_SERVICE;
}

/**
 * @internal
 * Creates an RTOS task.
 *
 * @param args  arguments corresponding to those of ICall_createTask().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when task creation failed.
 */
static ICall_Errno ICall_primCreateTask(ICall_CreateTaskArgs * args)
{
    /* Task_Params is a huge structure.
     * To reduce stack usage, heap is used instead.
     * This implies that ICall_createTask() must be called before heap
     * space may be exhausted.
     */
    Task_Params * params = (Task_Params *) ICall_heapMalloc(sizeof(Task_Params));
    Task_Handle task;

    if (params == NULL)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }

    Task_Params_init(params);
    params->priority  = args->priority;
    params->stackSize = args->stacksize;
    params->arg0      = args->arg;

    task = Task_create((Task_FuncPtr) args->entryfn, params, NULL);
    ICall_heapFree(params);

    if (task == NULL)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_JT */

#ifdef ICALL_RTOS_EVENT_API
/**
 * @internal
 * Creates an event.
 *
 * @param args  arguments corresponding to those of ICall_createEvent().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when task creation failed.
 */
static ICall_Errno ICall_primCreateEvent(ICall_CreateEventArgs * args)
{
    args->event = Event_create(NULL, NULL);

    if (args->event == NULL)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_RTOS_EVENT_API */
#ifdef ICALL_RTOS_SEMAPHORE_API
/**
 * @internal
 * Creates a semaphore.
 *
 * @param args  arguments corresponding to those of ICall_createSemaphore().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when task creation failed.
 */
static ICall_Errno ICall_primCreateSemaphore(ICall_CreateSemaphoreArgs * args)
{
    /* Semaphore_Params is a huge structure.
     * To reduce stack usage, heap is used instead.
     * This implies that ICall_createSemaphore() must be called before heap
     * space may be exhausted.
     */
    Semaphore_Params * semParams = (Semaphore_Params *) ICall_heapMalloc(sizeof(Semaphore_Params));

    if (semParams == NULL)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }

    Semaphore_Params_init(semParams);
    if (args->mode == ICALL_SEMAPHORE_MODE_BINARY)
    {
        semParams->mode = Semaphore_Mode_BINARY;
    }

    args->sem = Semaphore_create(args->initcount, semParams, NULL);
    ICall_heapFree(semParams);

    if (args->sem == NULL)
    {
        return ICALL_ERRNO_NO_RESOURCE;
    }
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_RTOS_SEMAPHORE_API */
#ifdef ICALL_RTOS_EVENT_API
/**
 * @internal
 * Waits on a ICALL_MSG_EVENT_ID.
 *
 * @param args  arguments corresponding to those of ICall_waitEvent().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when timeout occurred.
 */
static ICall_Errno ICall_primWaitEvent(ICall_WaitEventArgs * args)
{
    uint32_t timeout;

    if (args->milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        ICall_Errno errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return errno;
        }
    }

    if (Event_pend(args->event, 0, ICALL_MSG_EVENT_ID, timeout))
    {
        return ICALL_ERRNO_SUCCESS;
    }
    return ICALL_ERRNO_TIMEOUT;
}
#endif /* ICALL_RTOS_EVENT_API */

#ifdef ICALL_RTOS_SEMAPHORE_API
/**
 * @internal
 * Waits on a semaphore.
 *
 * @param args  arguments corresponding to those of ICall_waitSemaphore().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when timeout occurred.
 */
static ICall_Errno ICall_primWaitSemaphore(ICall_WaitSemaphoreArgs * args)
{
    uint32_t timeout;

    if (args->milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        ICall_Errno errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return errno;
        }
    }
    if (Semaphore_pend(args->sem, timeout))
    {
        return ICALL_ERRNO_SUCCESS;
    }
    return ICALL_ERRNO_TIMEOUT;
}
#endif /* ICALL_RTOS_SEMAPHORE_API */

#ifdef ICALL_RTOS_SEMAPHORE_API
/**
 * @internal signals a semaphore
 * @param args  arguments corresponding to those of ICall_signal()
 * @return return value corresponding to those of ICall_signal()
 */
static ICall_Errno ICall_primPostSemaphore(ICall_SignalArgs * args)
{
    Semaphore_post(args->syncHandle);
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_RTOS_EVENT_API */
#ifdef ICALL_RTOS_EVENT_API
/**
 * @internal signals an event
 * @param args  arguments corresponding to those of ICall_signal()
 * @return return value corresponding to those of ICall_signal()
 */
static ICall_Errno ICall_primPostEvent(ICall_SignalEventsArgs * args)
{
    Event_post(args->syncHandle, args->events);
    return ICALL_ERRNO_SUCCESS;
}
#endif /* ICALL_RTOS_EVENT_API */
/**
 * @internal Primitive service function ID to handler function map
 */
#ifndef ICALL_JT
static const struct _icall_primsvcfunc_map_entry_t
{
#ifdef COVERAGE_TEST
    size_t id;
#endif /* COVERAGE_TEST */
    ICall_PrimSvcFunc func;
} ICall_primSvcFuncs[] = {
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_ENROLL,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primEnroll },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_REGISTER_APP,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primRegisterApp },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_MSG_ALLOC,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primAllocMsg },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_MSG_FREE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primFreeMsg },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_MALLOC,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primMalloc },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_FREE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primFree },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_SEND_MSG,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primSend },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_FETCH_MSG,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primFetchMsg },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_SEND_SERV_MSG,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primSendServiceMsg },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_FETCH_SERV_MSG,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primFetchServiceMsg },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primWait },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_SIGNAL,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primSignal },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_ABORT,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primAbort },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_ENABLE_int,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primEnableint },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_DISABLE_int,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primDisableint },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_ENABLE_Mint,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primEnableMint },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_DISABLE_Mint,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primDisableMint },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_REGISTER_ISR,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primRegisterISR },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_GET_TICKS,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primGetTicks },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_SET_TIMER_MSECS,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primSetTimerMSecs },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_GET_TICK_PERIOD,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primGetTickPeriod },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_GET_MAX_MILLISECONDS,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primGetMaxMSecs },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_ENTITY2SERVICE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primE2S },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_UPD_ACTIVITY_COUNTER,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrUpdActivityCounter },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_REGISTER_NOTIFY,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrRegisterNotify },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT_MATCH,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primWaitMatch },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_GET_ENTITY_ID,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primGetEntityId },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_SET_TIMER,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primSetTimer },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_STOP_TIMER,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primStopTimer },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_CONFIG_AC_ACTION,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrConfigACAction },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_REQUIRE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrRequire },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_DISPENSE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrDispense },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_THREAD_SERVES,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primThreadServes },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_IS_STABLE_XOSC_HF,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrIsStableXOSCHF },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_GET_TRANSITION_STATE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrGetTransitionState },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_CREATE_TASK,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primCreateTask },

#ifdef ICALL_RTOS_SEMAPHORE_API
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_CREATE_SEMAPHORE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primCreateSemaphore },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT_SEMAPHORE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primWaitSemaphore },

#else /* ICALL_RTOS_SEMAPHORE_API */

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT_SEMAPHORE,
#endif /* COVERAGE_TEST */
        NULL },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT_SEMAPHORE,
#endif /* COVERAGE_TEST */
        NULL },
#endif /* ICALL_RTOS_SEMAPHORE_API */

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_SWITCH_XOSC_HF,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrSwitchXOSCHF },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_PWR_GET_XOSC_STARTUP_TIME,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICallPlatform_pwrGetXOSCStartupTime },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_REGISTER_ISR_EXT,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primRegisterISR_Ext },

#ifdef ICALL_RTOS_SEMAPHORE_API
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_POST_SEMAPHORE,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primPostSemaphore },
#else /*ICALL_RTOS_SEMAPHORE_API */
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_POST_SEMAPHORE,
#endif /* COVERAGE_TEST */
        NULL }, /* ICALL_RTOS_SEMAPHORE_API */
#endif

#ifdef ICALL_RTOS_EVENT_API
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_CREATE_EVENT,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primCreateEvent },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT_EVENT,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primWaitEvent },

    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_POST_EVENT,
#endif /* COVERAGE_TEST */
        (ICall_PrimSvcFunc) ICall_primPostEvent },
#else /*ICALL_RTOS_EVENT_API */
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_CREATE_EVENT,
#endif /* COVERAGE_TEST */
        NULL },
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_WAIT_EVENT,
#endif /* COVERAGE_TEST */
        NULL }, /* ICALL_RTOS_EVENT_API */
    {
#ifdef COVERAGE_TEST
        ICALL_PRIMITIVE_FUNC_POST_EVENT,
#endif /* COVERAGE_TEST */
        NULL }, /* ICALL_RTOS_EVENT_API */
#endif /* ICALL_RTOS_EVENT_API */
};
/**
 * @internal
 * Primitive service registered handler function
 * @param args   arguments
 * @return error code
 */
static ICall_Errno ICall_primService(ICall_FuncArgsHdr * args)
{
    if (args->func >= sizeof(ICall_primSvcFuncs) / sizeof(ICall_primSvcFuncs[0]))
    {
        return ICALL_ERRNO_INVALID_FUNCTION;
    }
    return ICall_primSvcFuncs[args->func].func(args);
}

/**
 * @internal Enrolls primitive service
 */
static void ICall_initPrim(void)
{
    ICall_entities[0].service = ICALL_SERVICE_CLASS_PRIMITIVE;
    ICall_entities[0].fn      = ICall_primService;

    /* Initialize heap */
    ICall_heapInit();

    /* TODO: Think about freezing permanently allocated memory blocks
     * for optimization.
     * Now that multiple stack images may share the same heap.
     * kick cannot be triggered by a single stack image.
     * Hence, maybe there should be an alternative API to
     * permanently allocate memory blocks, such as
     * by allocating the blocks at the end of the heap space. */
}
#endif /* ICALL_JT */

#ifdef COVERAGE_TEST
/**
 * @internal
 * Verification function for ICall implementation
 */
void ICall_verify(void)
{
    size_t i;
    for (i = 0; i < sizeof(ICall_primSvcFuncs) / sizeof(ICall_primSvcFuncs[0]); i++)
    {
        if (i != ICall_primSvcFuncs[i].id)
        {
            ICall_abort();
        }
    }
}
#endif /* COVERAGE_TEST */

#ifdef ICALL_JT
/**
 * Registers an application.
 * Note that this function must be called from the thread
 * from which ICall_wait() function will be called.
 *
 * @param entity  pointer to a variable to store entity id assigned
 *                to the application.
 * @param msgsem  pointer to a variable to store the synchronous object handle
 *                associated with the calling thread.
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 */
ICall_Errno ICall_registerApp(ICall_EntityID * entity, ICall_SyncHandle * msgSyncHdl)
{

    size_t i;
    ICall_TaskEntry * taskentry = ICall_newTask(Task_self());
    ICall_CSState key;

    if (!taskentry)
    {
        /* abort */
        ICALL_HOOK_ABORT_FUNC();
        return ICALL_ERRNO_NO_RESOURCE;
    }

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Use this entry */
            ICall_entities[i].service = ICALL_SERVICE_CLASS_APPLICATION;
            ICall_entities[i].task    = taskentry;
            ICall_entities[i].fn      = NULL;
            *entity                   = (ICall_EntityID) i;
            *msgSyncHdl               = taskentry->syncHandle;
            ICall_leaveCSImpl(key);
            return ICALL_ERRNO_SUCCESS;
        }
    }
    /* abort */
    ICALL_HOOK_ABORT_FUNC();
    ICall_leaveCSImpl(key);
    return (ICALL_ERRNO_NO_RESOURCE);
}

/**
 * Allocates memory block for a message.
 * @param size   size of the message body in bytes.
 * @return pointer to the start of the message body of the newly
 *         allocated memory block, or NULL if the allocation
 *         failed.
 */
void * ICall_allocMsg(size_t size)
{
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) ICall_heapMalloc(sizeof(ICall_MsgHdr) + size);

    if (!hdr)
    {
        return NULL;
    }
    hdr->len     = size;
    hdr->next    = NULL;
    hdr->dest_id = ICALL_UNDEF_DEST_ID;
    return ((void *) (hdr + 1));
}

/**
 * Frees the memory block allocated for a message.
 * @param msg   pointer to the start of the message body
 *              which was returned from ICall_allocMsg().
 */
void ICall_freeMsg(void * msg)
{
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) msg - 1;
    ICall_heapFree(hdr);
}

/**
 * Sends a message to a registered server.
 * @param src    Entity id of the sender of the message
 * @param dest   Service id
 * @param format Message format:
 *               @ref ICALL_MSG_FORMAT_KEEP,
 *               @ref ICALL_MSG_FORMAT_1ST_CHAR_TASK_ID or
 *               @ref ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID.
 *               Message format indicates whether and which
 *               field of the message must be transformed
 *               into a implementation specific sender
 *               identity for an external image.<br>
 *               When a service message interface is defined,
 *               it may contain a field that is not understood
 *               by the client but only understood by
 *               the system on the server's side.
 *               The format provides an information to the
 *               messaging system on such a server
 *               so that it can generically tag necessary
 *               information to the message.
 * @param msg    pointer to the message body to send.<br>
 *               Note that if message is successfully sent,
 *               the caller should not reference the message any
 *               longer.<br>
 *               However, if the function fails, the caller
 *               still owns the reference to the message.
 *               That is, caller may attempt another send,
 *               or may free the memory block, etc.
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_SERVICE when the 'dest'
 *         is unregistered service.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the 'src'
 *         is an out of range entity id or when 'dest' is
 *         is a service that does not receive a message
 *         (such as ICall primitive service).<br>
 *         Note that as far as 'src' is within the range,
 *         this function won't notice the 'src' entity id
 *         as invalid.
 */

ICall_Errno ICall_sendServiceMsg(ICall_EntityID src, ICall_ServiceEnum dest, ICall_MSGFormat format, void * msg)
{
    ICall_EntityID dstentity = ICall_searchServiceEntity(dest);

    if (dstentity == ICALL_INVALID_ENTITY_ID)
    {
        return ICALL_ERRNO_INVALID_SERVICE;
    }
    return (ICall_send(src, dstentity, format, msg));
}

/**
 * Retrieves a message received at the message queue
 * associated with the calling thread.
 *
 * Note that this function should be used by an application
 * which does not expect any message from non-server entity.
 *
 * @param src    pointer to a variable to store the service id
 *               of the registered server which sent the retrieved
 *               message
 * @param dest   pointer to a variable to store the entity id
 *               of the destination of the message.
 * @param msg    pointer to a pointer variable to store the
 *               starting address of the message body being
 *               retrieved.
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 *         and a message was retrieved.<br>
 *         @ref ICALL_ERRNO_NOMSG when there is no queued message
 *         at the moment.<br>
 *         @ref ICALL_ERRNO_CORRUPT_MSG when a message queued in
 *         front of the thread's receive queue was not sent by
 *         a server. Note that in this case, the message is
 *         not retrieved but thrown away.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when this function is
 *         called from a thread which has not registered
 *         an entity, either through ICall_enrollService()
 *         or through ICall_registerApp().
 */
ICall_Errno ICall_fetchServiceMsg(ICall_ServiceEnum * src, ICall_EntityID * dest, void ** msg)
{
    ICall_ServiceEnum servId;
    ICall_Errno errno = ICall_fetchMsg((ICall_EntityID *) src, dest, msg);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
        if (ICall_primEntityId2ServiceId(*src, &servId) != ICALL_ERRNO_SUCCESS)
        {
            /* Source entity ID cannot be translated to service id */
            ICall_freeMsg(*msg);
            return ICALL_ERRNO_CORRUPT_MSG;
        }
        *src = servId;
    }
    return (errno);
}
#if 0
void  convertMilliToTimepec(struct timespec *timeoutPosix,uint32_t timeout)
{
    timeoutPosix->tv_sec = (timeout / 1000);
    timeout = timeout % 1000;

    /* 1 millisecond = 1 000 000 nanoseconds */
    timeoutPosix->tv_nsec = timeout * 1000000;
}
#endif

/**
 * Waits for a signal to the semaphore associated with the calling thread.
 *
 * Note that the semaphore associated with a thread is signaled
 * when a message is queued to the message receive queue of the thread
 * or when ICall_signal() function is called onto the semaphore.
 *
 * @param milliseconds  timeout period in milliseconds.
 * @return @ref ICALL_ERRNO_SUCCESS when the semaphore is signaled.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the semaphore being signaled.
 */
ICall_Errno ICall_wait(uint_fast32_t milliseconds)
{
    TaskHandle_t taskhandle     = Task_self();
    ICall_TaskEntry * taskentry = ICall_searchTask(taskhandle);
    uint32_t timeout;
    uint32_t event;

    int16_t retVal = 0;

    if (!taskentry)
    {
        return (ICALL_ERRNO_UNKNOWN_THREAD);
    }
    /* Successful */
    if (milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        /* Convert milliseconds to number of ticks */
        ICall_Errno errno = ICall_msecs2Ticks(milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return (errno);
        }
    }

#ifdef FREERTOS
    if (HwiP_inISR())
    {
        xQueueReceiveFromISR(taskentry->syncHandle, (char *) &event, NULL);
    }
    else
    {
        xQueueReceive(taskentry->syncHandle, (char *) &event, milliseconds * 100);
    }
#endif
    if (retVal != (-1))
    {
        return (ICALL_ERRNO_SUCCESS);
    }

    return (ICALL_ERRNO_TIMEOUT);
}

/**
 * Signals a semaphore.
 * @param msgsem   handle of a synchronous object to signal
 * @return @ref ICALL_ERRNO_SUCCESS
 */
ICall_Errno ICall_signal(ICall_SyncHandle msgSyncHdl)
{
    /* 0x80000000 is an internal Event_ID */
    uint32_t msg_ptr   = 0x80000000;
    ICall_Errno status = ICALL_ERRNO_NO_RESOURCE;

#ifdef FREERTOS
    uint8_t statusQ;
    if (HwiP_inISR())
    {
        statusQ = xQueueSendFromISR(msgSyncHdl, (char *) &msg_ptr, NULL);
    }
    else
    {
        statusQ = xQueueSend(msgSyncHdl, (char *) &msg_ptr, 0);
    }

    if (statusQ == pdTRUE)
    {
        status = ICALL_ERRNO_SUCCESS;
    }
#endif
    return (status);
}

/**
 * Registers a service entity
 * @param service  service id of the enrolling service
 * @param fn       handler function which handles function
 *                 calls to the service.
 * @param entity   pointer to a variable to store the assigned entity id
 * @param msgsem   pointer to a variable to store the synchronous object handle
 *                 associated with the calling thread.
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when service id is already
 *         registered by another entity.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when maximum number of services
 *         are already registered.
 */
ICall_Errno ICall_enrollService(ICall_ServiceEnum service, ICall_ServiceFunc fn, ICall_EntityID * entity,
                                ICall_SyncHandle * msgSyncHdl)
{
    size_t i;
    ICall_TaskEntry * taskentry = ICall_newTask(Task_self());
    ICall_CSState key;

    /* Note that certain service does not handle a message
     * and hence, taskentry might be NULL.
     */
    if (taskentry == NULL)
    {
        return (ICALL_ERRNO_INVALID_PARAMETER);
    }

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Use this entry */
            ICall_entities[i].service = service;
            ICall_entities[i].task    = taskentry;
            ICall_entities[i].fn      = fn;
            *entity                   = (ICall_EntityID) i;
            *msgSyncHdl               = taskentry->syncHandle;

            ICall_leaveCSImpl(key);
            return (ICALL_ERRNO_SUCCESS);
        }
        else if (service == ICall_entities[i].service)
        {
            /* Duplicate service enrollment */
            ICall_leaveCSImpl(key);
            return (ICALL_ERRNO_INVALID_PARAMETER);
        }
    }
    /* abort */
    ICALL_HOOK_ABORT_FUNC();
    ICall_leaveCSImpl(key);
    return (ICALL_ERRNO_NO_RESOURCE);
}
#ifdef FREERTOS

/**
 * Allocates a memory block.
 * @param size   size of the block in bytes.
 * @return address of the allocated memory block or NULL
 *         if allocation fails.
 */

void * ICall_heapMalloc(uint32_t size)
{
    void * ret = NULL;
    ret        = malloc(size);
    return ret;
}

/**
 * Frees an allocated memory block.
 * @param msg  pointer to a memory block to free.
 */
void ICall_heapFree(void * msg)
{
    free(msg);
}

/**
 * Allocates a memory block, but check if enough memory will be left after the allocation.
 * @param size   size of the block in bytes.
 * @return address of the allocated memory block or NULL
 *         if allocation fails.
 */

void * ICall_heapMallocLimited(uint_least16_t size)
{
    return malloc(size);
}

/**
 * Get Statistic on Heap.
 * @param stats  pointer to a heapStats_t structure.
 */

/* Statistics currently are not supported via ICall apis.
 *  Please consider to use bget statistics (or any of your internal heap statistics)  */
void ICall_heapGetStats(ICall_heapStats_t * pStats) {}

#endif // FREERTOS
/**
 * Allocates a memory block.
 * @param size   size of the block in bytes.
 * @return address of the allocated memory block or NULL
 *         if allocation fails.
 */
void * ICall_malloc(uint_least16_t size)
{
    return (ICall_heapMalloc(size));
}

/**
 * Frees an allocated memory block.
 * @param msg  pointer to a memory block to free.
 */
void ICall_free(void * msg)
{
    ICall_heapFree(msg);
}

/**
 * Allocates a memory block, but check if enough memory will be left after the allocation.
 * @param size   size of the block in bytes.
 * @return address of the allocated memory block or NULL
 *         if allocation fails.
 */

void * ICall_mallocLimited(uint_least16_t size)
{
    return (ICall_heapMallocLimited(size));
}

/**
 * Get Statistic on Heap.
 * @param stats  pointer to a heapStats_t structure.
 */
void ICall_getHeapStats(ICall_heapStats_t * pStats)
{
    ICall_heapGetStats(pStats);
}

#ifdef HEAPMGR_METRICS
/**
 * @brief   obtain heap usage metrics
 * @param   pBlkMax   pointer to a variable to store max cnt of all blocks ever seen at once
 * @param   pBlkCnt   pointer to a variable to store current cnt of all blocks
 * @param   pBlkFree  pointer to a variable to store current cnt of free blocks
 * @param   pMemAlo   pointer to a variable to store current total memory allocated
 * @param   pMemMax   pointer to a variable to store max total memory ever allocated at once
 * @param   pMemUB    pointer to a variable to store the upper bound of memory usage
 */
void ICall_getHeapMgrGetMetrics(uint32_t * pBlkMax, uint32_t * pBlkCnt, uint32_t * pBlkFree, uint32_t * pMemAlo, uint32_t * pMemMax,
                                uint32_t * pMemUB)
{
    ICall_heapMgrGetMetrics(pBlkMax, pBlkCnt, pBlkFree, pMemAlo, pMemMax, pMemUB);
}

#endif
/**
 * Sends a message to an entity.
 * @param src     entity id of the sender
 * @param dest    entity id of the destination of the message.
 * @param format  message format. See ICall_sendServiceMsg().
 * @param msg     pointer to the message body.
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when either src
 *              or dest is not a valid entity id or when
 *              dest is an entity id of an entity that does
 *              not receive a message
 *              (e.g., ICall primitive service entity).
 */
ICall_Errno ICall_send(ICall_EntityID src, ICall_EntityID dest, ICall_MSGFormat format, void * msg)
{
    ICall_CSState key;
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) msg - 1;

    if (dest >= ICALL_MAX_NUM_ENTITIES || src >= ICALL_MAX_NUM_ENTITIES)
    {
        return (ICALL_ERRNO_INVALID_PARAMETER);
    }
    key = ICall_enterCSImpl();
    if (!ICall_entities[dest].task)
    {
        ICall_leaveCSImpl(key);
        return (ICALL_ERRNO_INVALID_PARAMETER);
    }

    ICall_leaveCSImpl(key);
    /* Note that once the entry is valid,
     * the value does not change and hence it is OK
     * to leave the critical section.
     */

    hdr->srcentity = src;
    hdr->dstentity = dest;
    hdr->format    = format;

    ICall_msgEnqueue(&ICall_entities[dest].task->queue, msg);
    /* 0x80000000 is an internal event number */
    uint32_t msg_ptr = 0x80000000;
#ifdef FREERTOS
    uint8_t status;

    if (HwiP_inISR())
    {
        status = xQueueSendFromISR(ICall_entities[dest].task->syncHandle, (char *) &msg_ptr, NULL);
    }
    else
    {
        status = xQueueSend(ICall_entities[dest].task->syncHandle, (char *) &msg_ptr, 0);
    }

    if (status != pdPASS)
    {
        return status;
    }
#endif
    return (ICALL_ERRNO_SUCCESS);
}

/**
 * Retrieves a message, queued to receive queue of the calling thread.
 *
 * @param src   pointer to a variable to store the sender entity id
 *              of the received message.
 * @param dest  pointer to a variable to store the destination entity id
 *              of the received message.
 * @param msg   pointer to a pointer variable to store the starting
 *              address of a received message body.
 * @return @ref ICALL_ERRNO_SUCCESS when a message was successfully
 *         retrieved.<br>
 *         @ref ICALL_ERRNO_NOMSG when no message was queued to
 *         the receive queue at the moment.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when the calling thread
 *         does not have a received queue associated with it.
 *         This happens when neither ICall_enrollService() nor
 *         ICall_registerApp() was ever called from the calling
 *         thread.
 */
ICall_Errno ICall_fetchMsg(ICall_EntityID * src, ICall_EntityID * dest, void ** msg)
{
    void * msgTemp;
    TaskHandle_t taskhandle     = Task_self();
    ICall_TaskEntry * taskentry = ICall_searchTask(taskhandle);
    ICall_MsgHdr * hdr;

    if (!taskentry)
    {
        return (ICALL_ERRNO_UNKNOWN_THREAD);
    }
    /* Successful */
    msgTemp = ICall_msgDequeue(&taskentry->queue);

    if (msgTemp == NULL)
    {
        return (ICALL_ERRNO_NOMSG);
    }
    hdr   = (ICall_MsgHdr *) msgTemp - 1;
    *src  = hdr->srcentity;
    *dest = hdr->dstentity;
    *msg  = msgTemp;
    return (ICALL_ERRNO_SUCCESS);
}

/**
 * Transforms and entityId into a serviceId.
 * Note that this function is useful in case an application
 * waits for messages from both a server and another application,
 * in which case, the application can only use ICall_fetchMsg(),
 * not ICall_fetchServiceMsg() because the latter will return
 * @ref ICALL_ERRNO_CORRUPT_MSG when a message sent by the other
 * application is about to be fetched.<br>
 * This function, then, is useful to retrieve service id
 * matching the source entity id in case the source entity
 * id is not that of the other application.
 *
 * @param entityId   entity id
 * @param servId     pointer to a variable to store
 *                   the resultant service id
 * @return @ref ICALL_ERRNO_SUCCESS if the transformation was successful.<br>
 *         @ref ICALL_ERRNO_INVALID_SERVICE if no matching service
 *         is found for the entity id.
 */
ICall_Errno ICall_entityId2ServiceId(ICall_EntityID entityId, ICall_ServiceEnum * servId)
{
    return ICall_primEntityId2ServiceId(entityId, servId);
}

/**
 * Aborts.
 *
 * This is preferred over C runtime abort() function,
 * in an external image since the C runtime abort() is only
 * guaranteed in a root image which contains the C runtime
 * entry function that is executed upon startup.
 */
ICall_Errno ICall_abort(void)
{
#ifdef HALNODEBUG
#elif defined(EXT_HAL_ASSERT)
    HAL_ASSERT(HAL_ASSERT_CAUSE_ICALL_ABORT);
#else
    {
        volatile uint8_t j = 1;
        while (j)
            ;
    }
#endif /* EXT_HAL_ASSERT */
    ICALL_HOOK_ABORT_FUNC();
    return (ICALL_ERRNO_SUCCESS);
}

/**
 * Enables interrupt.
 * @param intnum   interrupt number
 * @return @ref ICALL_ERRNO_SUCCESS.
 */
ICall_Errno ICall_enableint(int intnum)
{
    Hwi_enableinterrupt(intnum);
    return (ICALL_ERRNO_SUCCESS);
}

/**
 * Disables interrupt
 * @param intnum  interrupt number
 * @return @ref ICALL_ERRNO_SUCCESS
 */
ICall_Errno ICall_disableint(int intnum)
{
    Hwi_disableinterrupt(intnum);
    return (ICALL_ERRNO_SUCCESS);
}

/**
 * Gets the current tick counter value.
 * @return current tick counter value
 */
uint_fast32_t ICall_getTicks(void)
{
    return (ClockP_getSystemTicks());
}

/**
 * Gets the tick period.
 * @return tick period in microseconds.
 */
uint_fast32_t ICall_getTickPeriod(void)
{
    return CLOCK_TICKS_PERIOD;
}

/**
 * Gets the maximum timeout period supported by
 * ICall_setTimerMSecs() function.
 *
 * @return maximum timeout period in milliseconds
 */
uint_fast32_t ICall_getMaxMSecs(void)
{

    uint_fast64_t tmp = ((uint_fast64_t) 0x7ffffffful) * (ICall_getTickPeriod());
    tmp /= 1000;
    if (tmp >= 0x80000000ul)
    {
        tmp = 0x7ffffffful;
    }
    return ((uint_least32_t) tmp);
}

/**
 * Set up or restart a timer.
 * Note that the timer setup initially by this function may be restarted
 * using ICall_setTimer() function as well.
 *
 * @param msecs  timeout period in milliseconds after which callback function
 *               shall be called.
 * @param cback  callback function pointer
 * @param arg    argument to pass to the callback function
 * @param id     pointer to the timer ID.
 *               If a new timer must be set up, the value of the timer ID
 *               must be set to @ref ICALL_INVALID_TIMER_ID when making this
 *               call and when the function returns successfully, the variable
 *               will be assigned a new timer ID value.
 *               The value other than @ref ICALL_INVALID_TIMER_ID shall be
 *               regarded as a request to restart the earlier set timer.
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when msecs is greater than
 *              maximum value supported;<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 *         Check ICall heap size and OS heap size if this happens.
 *
 * @see ICall_getMaxMSecs()
 */
ICall_Errno ICall_setTimerMSecs(uint_fast32_t msecs, ICall_TimerCback cback, void * arg, ICall_TimerID * id)
{
    uint32_t ticks;
    uint32_t timeout;
    /* Convert to tick time */
    ICall_Errno errno = ICall_msecs2Ticks(msecs, &ticks);

    if (errno != ICALL_ERRNO_SUCCESS)
    {
        return (errno);
    }
    timeout = ticks;
    ICall_setTimer(timeout, cback, arg, id);

    return (errno);
}

/**
 * Set up or restart a timer.
 * Note that the timer setup initially by this function may be restarted
 * using ICall_setTimerMSecs() function as well.
 *
 * @param ticks  timeout period in ticks after which the callback function
 *               shall be called.
 * @param cback  callback function pointer
 * @param arg    argument to pass to the callback function
 * @param id     pointer to the timer ID.
 *               If a new timer must be set up, the value of the timer ID
 *               must be set to @ref ICALL_INVALID_TIMER_ID when making this
 *               call and when the function returns successfully, the variable
 *               will be assigned a new timer ID value.
 *               The value other than @ref ICALL_INVALID_TIMER_ID shall be
 *               regarded as a request to restart the earlier set timer.
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 *         Check ICall heap size and OS heap size if this happens.
 *
 * @see ICall_getTickPeriod()
 */

ICall_Errno ICall_setTimer(uint32_t ticks, ICall_TimerCback cback, void * arg, ICall_TimerID * id)
{

    ICall_ScheduleEntry * entry;

    if (*id == ICALL_INVALID_TIMER_ID)
    {
        ClockP_Params params;

        /* Create a new timer */
        entry = ICall_heapMalloc(sizeof(ICall_ScheduleEntry));
        if (entry == NULL)
        {
            /* allocation failed */
            return (ICALL_ERRNO_NO_RESOURCE);
        }
        ClockP_Params_init(&params);
        params.startFlag = FALSE;
        params.period    = 0;
        params.arg       = (uintptr_t) entry;

        entry->clockP = ClockP_create(ICall_clockFunc, ticks, &params);
        if (!entry->clockP)
        {
            /* abort */
            ICall_abort();
            ICall_heapFree(entry);
            return (ICALL_ERRNO_NO_RESOURCE);
        }
        entry->cback = cback;
        entry->arg   = arg;
        *id          = (ICall_TimerID) entry;
    }
    else
    {
        ICall_CSState key;

        entry = (ICall_ScheduleEntry *) *id;

        /* Critical section is entered to disable interrupts that might cause call
         * to callback due to race condition */
        key = ICall_enterCriticalSection();
        ClockP_stop(entry->clockP);
        entry->arg = arg;
        ICall_leaveCriticalSection(key);
    }
    ClockP_setTimeout(entry->clockP, ticks);

    ClockP_start(entry->clockP);

    return (ICALL_ERRNO_SUCCESS);
}

/**
 * Stops a timer.
 *
 * @param id    timer ID.
 */
void ICall_stopTimer(ICall_TimerID id)
{
    ICall_ScheduleEntry * entry = (ICall_ScheduleEntry *) id;

    if (id == ICALL_INVALID_TIMER_ID)
    {
        return;
    }

    ClockP_stop(entry->clockP);
}

/**
 * Increments or decrements power activity counter.
 *
 * When power activity counter is greater than zero,
 * the device shall stay in the active power state.
 * The caller has to make sure that it decrements the counter
 * as many times as it has incremented the counter
 * when there is no activity that requires the active power state.
 * It is recommended that each client increments the counter by just one,
 * but it is not mandated to be so.
 *
 * @param incFlag   TRUE to indicate incrementing the counter.<br>
 *                  FALSE to indicate decrementing the counter.
 * @return @ref TRUE if power is required.<br>
 *         @ref FALSE if power is not required.<br>
 */
bool ICall_pwrUpdActivityCounter(bool incFlag)
{
    ICall_PwrUpdActivityCounterArgs args;
    args.incFlag = incFlag;
    ICallPlatform_pwrUpdActivityCounter(&args);
    return (args.pwrRequired);
}

/**
 * Configures power constraint/dependency set/release actions upon
 * activity counter change.
 *
 * When activity counter value drops to zero, all constraints and
 * dependencies configured by this function shall be released.<br>
 * When activity counter value increments to one, all constraints
 * and dependencies configured by this function shall be set.
 *
 * @param bitmap  a bitmap of constraint or dependency flags.<br>
 *                The flag definition is specific to each platform.
 *                For instance, see ICallCC26xxDefs.h.
 *
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when an invalid
 *              flag in the bitmap is detected.<br>
 */
ICall_Errno ICall_pwrConfigACAction(ICall_PwrBitmap_t bitmap)
{
    ICall_PwrBitmapArgs args;
    args.bitmap = bitmap;
    return (ICallPlatform_pwrConfigACAction(&args));
}

/**
 * Sets power constraints and dependencies.
 *
 * @param bitmap  a bitmap of constraint or dependency flags.<br>
 *                The flag definition is specific to each platform.
 *                For instance, see ICallCC26xxDefs.h.
 *
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when an invalid
 *              flag in the bitmap is detected.<br>
 */
ICall_Errno ICall_pwrRequire(ICall_PwrBitmap_t bitmap)
{
    ICall_PwrBitmapArgs args;
    args.bitmap = bitmap;
    return (ICallPlatform_pwrRequire(&args));
}

/**
 * Releases power constraints and dependencies.
 *
 * @param bitmap  a bitmap of constraint or dependency flags.<br>
 *                The flag definition is specific to each platform.
 *                For instance, see ICallCC26xxDefs.h.
 *
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when an invalid
 *              flag in the bitmap is detected.<br>
 */
ICall_Errno ICall_pwrDispense(ICall_PwrBitmap_t bitmap)
{
    ICall_PwrBitmapArgs args;
    args.bitmap = bitmap;
    return (ICallPlatform_pwrDispense(&args));
}

/**
 * Checks whether HF XOSC is stable.
 * This function must be called after HF XOSC is turned on
 * (through power dependency).
 *
 * @return TRUE when HF XOSC is stable.<br>
 *         FALSE when HF XOSC is not stable.<br>
 */
bool ICall_pwrIsStableXOSCHF(void)
{
    ICall_GetBoolArgs args;
    (void) ICallPlatform_pwrIsStableXOSCHF(&args);
    return (args.value);
}

/**
 * Switch clock source to HF XOSC.
 * This function must be called after HF XOSC is stable.
 *
 * @return @ref ICALL_ERRNO_SUCCESS
 */
ICall_Errno ICall_pwrSwitchXOSCHF(void)
{
    ICall_FuncArgsHdr args;
    return (ICallPlatform_pwrSwitchXOSCHF(&args));
}

/**
 * Gets the estimated crystal oscillator startup time.
 *
 * @return estimated crystal oscillator startup time
 */
uint32_t ICall_pwrGetXOSCStartupTime(uint_fast32_t timeUntilWakeupInMs)
{
    ICall_PwrGetXOSCStartupTimeArgs args;
    args.timeUntilWakeupInMs = timeUntilWakeupInMs;
    (void) ICallPlatform_pwrGetXOSCStartupTime(&args);
    return (args.value);
}

/**
 * Registers a power state transition notify function.
 *
 * The registered notify function shall be called when the power state
 * changes.
 *
 * @param fn  notify function pointer
 * @param obj pointer to data object to be passed to notify function.<br>
 *            This pointer must not be NULL and can point to an aggregate type
 *            containing the @ref ICall_PwrNotifyData.
 *            The object must not be released.
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when registration failed<br>
 */
ICall_Errno ICall_pwrRegisterNotify(ICall_PwrNotifyFn fn, ICall_PwrNotifyData * obj)
{
    ICall_PwrRegisterNotifyArgs args;
    args.hdr.service = ICALL_SERVICE_CLASS_PRIMITIVE;
    args.hdr.func    = ICALL_PRIMITIVE_FUNC_PWR_REGISTER_NOTIFY;
    args.fn          = fn;
    args.obj         = obj;
    return (ICallPlatform_pwrRegisterNotify(&args));
}

/**
 * Retrieves power transition state.
 *
 * @return Implementation specific transition state when successful<br>
 *         Zero when the function is not implemented.<br>
 */
uint_fast8_t ICall_pwrGetTransitionState(void)
{
    ICall_PwrGetTransitionStateArgs args;
    ICall_Errno errno;

    errno = ICallPlatform_pwrGetTransitionState(&args);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
        return (args.state);
    }
    return (0);
}

/**
 * Creates a new RTOS task.
 *
 * @param entryfn   task entry function.
 * @param priority  task priority as understood by the underlying RTOS
 * @param stacksize stack size as understood by the underlying RTOS
 * @param arg       argument to pass to the task entry function
 *
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when creation failed<br>
 */

#ifdef ICALL_RTOS_SEMAPHORE_API
/**
 * Creates a semaphore.
 *
 * @param mode  Semaphore mode<br>
 *              @ref ICALL_SEMAPHORE_MODE_COUNTING, or<br>
 *              @ref ICALL_SEMAPHORE_MODE_BINARY
 * @param initcount initial count value
 * @return created semaphore when successful<br>
 *         NULL when creation failed<br>
 */
ICall_Semaphore ICall_createSemaphore(uint_fast8_t mode, int initcount)
{
    /* Semaphore_Params is a huge structure.
     * To reduce stack usage, heap is used instead.
     * This implies that ICall_createSemaphore() must be called before heap
     * space may be exhausted.
     */
    ICall_Semaphore sem;
    Semaphore_Params * semParams = (Semaphore_Params *) ICall_heapMalloc(sizeof(Semaphore_Params));

    if (semParams == NULL)
    {
        return (NULL);
    }

    Semaphore_Params_init(semParams);
    if (mode == ICALL_SEMAPHORE_MODE_BINARY)
    {
        semParams->mode = Semaphore_Mode_BINARY;
    }

    sem = Semaphore_create(args->initcount, semParams, NULL);
    ICall_heapFree(semParams);

    return (sem);
}
#endif

#ifdef ICALL_RTOS_SEMAPHORE_API
/**
 * Post on a semaphore
 *
 * @param sem           semaphore.
 *
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 */
ICall_Errno ICall_postSemaphore(ICall_Semaphore sem)
{
    Semaphore_post(sem);
    return (ICALL_ERRNO_SUCCESS);
}
#endif /* ICALL_RTOS_SEMAPHORE_API */

#ifdef ICALL_RTOS_EVENTS_API
/**
 * Creates a event.
 *
 * @return created event when successful<br>
 *         NULL when creation failed<br>
 */
ICall_Event ICall_createEvent(void)
{
    ICall_Event event = Event_create(NULL, NULL);
    return (event);

    ICall_CreateEventArgs args;
    ICall_Errno errno;

    args.hdr.service = ICALL_SERVICE_CLASS_PRIMITIVE;
    args.hdr.func    = ICALL_PRIMITIVE_FUNC_CREATE_EVENT;
    errno            = ICall_dispatcher(&args.hdr);
    if (errno == ICALL_ERRNO_SUCCESS)
    {
        return (args.event);
    }
    return (NULL);
}

/**
 * Post on an event
 *
 * @param event           event.
 *
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 */
ICall_Errno ICall_postEvent(ICall_Event event, uint32_t events)
{
    Event_post(event, events);
    return (ICALL_ERRNO_SUCCESS);
}
/**
 * Waits on a event for ICALL_MSG_EVENT_ID
 *
 * @param event           event.
 * @param milliseconds  timeout in milliseconds
 *                      or @ref ICALL_TIMEOUT_FOREVER to wait forever
 *
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 *         and event was received.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the event being signaled.
 */
ICall_Errno ICall_waitEvent(ICall_Event event, uint_fast32_t milliseconds)
{
    uint32_t timeout;

    if (milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        ICall_Errno errno = ICall_msecs2Ticks(milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return (errno);
        }
    }

    if (Event_pend(event, 0, ICALL_MSG_EVENT_ID, timeout))
    {
        return (ICALL_ERRNO_SUCCESS);
    }
    return (ICALL_ERRNO_TIMEOUT);
}
#endif /* ICALL_RTOS_EVENTS_API */

#ifdef ICALL_RTOS_SEMAPHORE_API
/**
 * Waits on a semaphore
 *
 * @param sem           semaphore.
 * @param milliseconds  timeout in milliseconds
 *                      or @ref ICALL_TIMEOUT_FOREVER to wait forever
 *
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 *         and semaphore was acquired.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the semaphore being signaled.
 */
ICall_Errno ICall_waitSemaphore(ICall_Semaphore sem, uint_fast32_t milliseconds)
{
    uint32_t timeout;

    if (milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        ICall_Errno errno = ICall_msecs2Ticks(milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return (errno);
        }
    }
    if (Semaphore_pend(sem, timeout))
    {
        return (ICALL_ERRNO_SUCCESS);
    }
    return (ICALL_ERRNO_TIMEOUT);
}
#endif /* ICALL_RTOS_SEMAPHORE_API */

#if 0
/* Util function that take time in ticks and convert it into ms - relate to system clock (returns system clock + converted ms) */
static void AbsoluteTimeInMilliPlusTimer(uint_least32_t timeout,struct timespec *tsTimer)
{

    clock_gettime(CLOCK_REALTIME, tsTimer);

    tsTimer->tv_sec += (timeout / 1000);
    tsTimer->tv_nsec += (timeout % 1000) * 1000000;
}
#endif

/**
 * Waits for and retrieves a message received at the message queue
 * associated with the calling thread, which matches a certain condition.
 *
 * @param milliseconds  timeout period in milliseconds.
 * @param matchFn  pointer to a function that would return TRUE when
 *                 the message matches its condition.
 * @param src    pointer to a variable to store the service id
 *               of the registered server which sent the retrieved
 *               message, or NULL if not interested in storing service id.
 * @param dest   pointer to a variable to store the entity id
 *               of the destination of the message,
 *               of NULL if not interested in storing the destination entity id.
 * @param msg    pointer to a pointer variable to store the
 *               starting address of the message body being
 *               retrieved. The pointer must not be NULL.
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 *         and a message was retrieved.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the semaphore being signaled.
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when this function is
 *         called from a thread which has not registered
 *         an entity, either through ICall_enrollService()
 *         or through ICall_registerApp().
 */
ICall_Errno ICall_waitMatch(uint_least32_t milliseconds, ICall_MsgMatchFn matchFn, ICall_ServiceEnum * src, ICall_EntityID * dest,
                            void ** msg)
{
    TaskHandle_t taskhandle     = Task_self();
    ICall_TaskEntry * taskentry = ICall_searchTask(taskhandle);
    ICall_MsgQueue prependQueue = NULL;
#ifndef ICALL_EVENTS
    uint_fast16_t consumedCount = 0;
#endif
    uint32_t timeout;
    uint_fast32_t timeoutStamp;
    ICall_Errno errno;

    if (!taskentry)
    {
        return (ICALL_ERRNO_UNKNOWN_THREAD);
    }
    /* Successful */
    if (milliseconds == 0)
    {
        timeout = BIOS_NO_WAIT;
    }
    else if (milliseconds == ICALL_TIMEOUT_FOREVER)
    {
        timeout = BIOS_WAIT_FOREVER;
    }
    else
    {
        /* Convert milliseconds to number of ticks */
        errno = ICall_msecs2Ticks(milliseconds, &timeout);
        if (errno != ICALL_ERRNO_SUCCESS)
        {
            return (errno);
        }
    }

    errno = ICALL_ERRNO_TIMEOUT;

    timeoutStamp = ICall_getTicks() + timeout;

#ifdef ICALL_LITE

    uint32_t events;

#ifdef FREERTOS
    // TODO: Investigate ICALL Wait tick period (Last parameter)
    while (xQueueReceive(taskentry->syncHandle, (char *) &events, milliseconds * 1000) == pdPASS)
#endif
#else  /* !ICALL_LITE */
    while (ICALL_SYNC_HANDLE_PEND(taskentry->syncHandle, timeout))
#endif /* ICALL_LITE */
    {
        ICall_EntityID fetchSrc;
        ICall_EntityID fetchDst;
        ICall_ServiceEnum servId;
        void * fetchMsg;
        errno = ICall_fetchMsg(&fetchSrc, &fetchDst, &fetchMsg);
        if (errno == ICALL_ERRNO_SUCCESS)
        {
            if (ICall_primEntityId2ServiceId(fetchSrc, &servId) == ICALL_ERRNO_SUCCESS)
            {
                if (matchFn(servId, fetchDst, fetchMsg))
                {
                    /* Matching message found*/
                    if (src != NULL)
                    {
                        *src = servId;
                    }
                    if (dest != NULL)
                    {
                        *dest = fetchDst;
                    }
                    *msg  = fetchMsg;
                    errno = ICALL_ERRNO_SUCCESS;
                    break;
                }
            }
            /* Message was received but it wasn't expected one.
             * Add to the prepend queue */
            ICall_msgEnqueue(&prependQueue, fetchMsg);
#ifdef ICALL_EVENTS
            /* Event are binary semaphore, so if several messsages are posted while
             * we are processing one, it's possible that some of them are 'missed' and
             * not processed. Sending a event to ourself force this loop to run until
             * all the messages in the queue are processed.
             */
#ifdef ICALL_LITE
            /* 0x20000000 is an internal Event_ID */
            uint32_t msg_ptr = (0x20000000); // Event_Id_29;

#ifdef FREERTOS

            uint8_t status;
            if (HwiP_inISR())
            {
                status = xQueueSendFromISR(taskentry->syncHandle, (char *) &msg_ptr, NULL);
            }
            else
            {
                status = xQueueSend(taskentry->syncHandle, (char *) &msg_ptr, 0);
            }

            if (status != pdTRUE)
            {
                return status;
            }
#endif

#else  /* !ICALL_LITE */
            ICALL_SYNC_HANDLE_POST(taskentry->syncHandle);
#endif /* ICALL_LITE*/
#endif /* ICALL_EVENTS */
        }

        /* Prepare for timeout exit */
        errno = ICALL_ERRNO_TIMEOUT;

#ifndef ICALL_EVENTS
        /* Keep the decremented semaphore count */
        consumedCount++;
#endif /* ICALL_EVENTS */
        if (timeout != BIOS_WAIT_FOREVER && timeout != BIOS_NO_WAIT)
        {
            /* Readjust timeout */
            uint32_t newTimeout = timeoutStamp - ICall_getTicks();

            if (newTimeout == 0 || newTimeout > timeout)
            {
                break;
            }
            timeout = newTimeout;
        }
    }

    /* Prepend retrieved irrelevant messages */
    ICall_msgPrepend(&taskentry->queue, prependQueue);
#ifndef ICALL_EVENTS
    /* Re-increment the consumed semaphores */
    for (; consumedCount > 0; consumedCount--)
    {
        Semaphore_post(taskentry->syncHandle);
    }
#endif /* ICALL_EVENTS */
    return (errno);
}

/**
 * Retrieves an entity ID of (arbitrary) one of the entities registered
 * from the calling thread.
 *
 * Note that, if multiple entities were registered from the same thread,
 * this function shall arbitrarily pick one of the entities.
 *
 * @return A valid entity ID or @ref ICALL_INVALID_ENTITY_ID
 *         when no entity was registered from the calling thread.
 */
ICall_EntityID ICall_getEntityId(void)
{
    ICall_EntityID id;
    TaskHandle_t taskhandle = Task_self();
    ICall_CSState key;
    size_t i;
    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Not found */
            break;
        }
        if (ICall_entities[i].task->task == (TaskHandle_t) taskhandle)
        {
            /* Found */
            id = i;
            ICall_leaveCSImpl(key);
            return (id);
        }
    }
    ICall_leaveCSImpl(key);
    return ICALL_INVALID_ENTITY_ID;
}

/**
 * Checks whether the calling thread provides the designated service.
 *
 * @param  service    Service enumeration value for the service.
 *
 * @return Non-zero if the current thread provides the designated service.
 *         Zero, otherwise.
 */
uint_fast8_t ICall_threadServes(ICall_ServiceEnum service)
{
    uint_fast8_t res = 0;
    TaskHandle_t taskhandle;
    ICall_CSState key;
    size_t i;
    taskhandle = Task_self();

    key = ICall_enterCSImpl();
    for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
    {
        if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
        {
            /* Not found */
            break;
        }
        else if (ICall_entities[i].service == service)
        {
            res = (uint_fast8_t) (ICall_entities[i].task->task == taskhandle);
        }
    }
    ICall_leaveCSImpl(key);
    return (res);
}

/**
 * Retrieves an internal ID understood only by a service entity,
 * corresponding to an ICall entity ID.
 *
 * This function is used when a message body includes a field indicating a
 * software entity and the destination of the message has its own internal
 * message routing mechanism with its own entity enumeration system.
 *
 * @param  service    Service enumeration value for the service.
 *                    Currently @ref ICALL_SERVICE_CLASS_BLE_MSG is supported.
 * @param  entity     ICall entity ID.
 *
 * @return Stack specific 8 bit ID or 0xFF when failed.
 */
uint_fast8_t ICall_getLocalMsgEntityId(ICall_ServiceEnum service, ICall_EntityID entity)
{
    ICall_GetLocalMsgEntityIdArgs args;
    ICall_Errno errno;
    args.hdr.service = service;
    args.hdr.func    = ICALL_MSG_FUNC_GET_LOCAL_MSG_ENTITY_ID;
    args.entity      = entity;
    errno            = ICall_dispatcher(&args.hdr);
    if (errno == ICALL_ERRNO_SUCCESS)
    {
        return (args.localId);
    }
    return (0xFF);
}

#endif /* ICALL_JT */

#ifdef ICALL_LITE
/*******************************************************************************
 * @fn          matchLiteCS
 */
static bool matchLiteCS(ICall_ServiceEnum src, ICall_EntityID dest, const void * msg)
{
    (void) src;
    (void) dest;
    ICall_LiteCmdStatus * pMsg = (ICall_LiteCmdStatus *) msg;
    return (pMsg->cmdId == ICALL_LITE_DIRECT_API_DONE_CMD_ID);
}
/*******************************************************************************
 * @fn          icall_directAPI
 * see headers for details.
 */
uint32_t icall_directAPI(uint8_t service, icall_lite_id_t id, ...)
{
    va_list argp;
    uint32_t res;
    icallLiteMsg_t liteMsg;

    // The following will push all parameter in the runtime stack.
    // This need to be call before any other local declaration of variable....
    va_start(argp, id);

    // Todo - add string for every icall API function, instead of printing function address
    BLE_LOG_INT_INT(0, BLE_LOG_MODULE_APP, "APP : icall_directAPI to BLE func=0x%x, status=%d\n", id, 0);
    // Create the message that will be send to the requested service..
    liteMsg.hdr.len          = sizeof(icallLiteMsg_t);
    liteMsg.hdr.next         = NULL;
    liteMsg.hdr.dest_id      = ICALL_UNDEF_DEST_ID;
    liteMsg.msg.directAPI    = id;
    liteMsg.msg.pointerStack = (uint32_t *) (*((uint32_t *) (&argp)));
    ICall_sendServiceMsg(ICall_getEntityId(), service, ICALL_MSG_FORMAT_DIRECT_API_ID, &(liteMsg.msg));

    // Since stack needs to always have a higher priority than the thread calling
    // the API, when we reach this point the API has been executed by the stack.
    // This implies the following:
    //  - API are not called in critical section or in section where task
    //    switching is disabled
    // It is possible that the stack is blocking on this API, in this case a
    // sync object needs to be used in order for this call to resume only when
    // the API has been process in full.
    {
        ICall_Errno errno;
        void * pCmdStatus = NULL;

        errno = ICall_waitMatch(ICALL_TIMEOUT_PREDEFINE, matchLiteCS, NULL, NULL, (void **) &pCmdStatus);
        if (errno == ICALL_ERRNO_TIMEOUT)
        {
#ifdef HALNODEBUG
#elif defined(EXT_HAL_ASSERT)
            HAL_ASSERT(HAL_ASSERT_CAUSE_ICALL_TIMEOUT);
#else  /* !EXT_HAL_ASSERT */
            ICall_abort();
#endif /* EXT_HAL_ASSERT */
        }
        else if (errno == ICALL_ERRNO_SUCCESS)
        {
            if (pCmdStatus)
            {
                ICall_freeMsg(pCmdStatus);
            }
        }
        else
        {
#ifdef HALNODEBUG
#else  /* ! HALNODEBUG */
            ICall_abort();
#endif /* HALNODEBUG */
        }
    }

    // The return parameter is set in the runtime stack, at the location of the
    // first parameter.
    res = liteMsg.msg.pointerStack[0];

    va_end(argp);

    return (res);
}

/*******************************************************************************
 * @fn          ICall_sendServiceComplete
 * see headers for details.
 */
ICall_Errno ICall_sendServiceComplete(ICall_EntityID src, ICall_EntityID dest, ICall_MSGFormat format, void * msg)
{
    ICall_CSState key;
    ICall_MsgHdr * hdr = (ICall_MsgHdr *) msg - 1;

    if (dest >= ICALL_MAX_NUM_ENTITIES || src >= ICALL_MAX_NUM_ENTITIES)
    {
        return (ICALL_ERRNO_INVALID_PARAMETER);
    }
    key = ICall_enterCSImpl();
    if (!ICall_entities[dest].task)
    {
        ICall_leaveCSImpl(key);
        return (ICALL_ERRNO_INVALID_PARAMETER);
    }

    ICall_leaveCSImpl(key);
    /* Note that once the entry is valid,
     * the value does not change and hence it is OK
     * to leave the critical section.
     */

    hdr->srcentity = src;
    hdr->dstentity = dest;
    hdr->format    = format;
    ICall_msgEnqueue(&ICall_entities[dest].task->queue, msg);

    /* 0x20000000 is an internal Event_ID */
    uint32_t msg_ptr = (0x20000000); // Event_Id_29;
#ifdef FREERTOS
    uint8_t status;
    if (HwiP_inISR())
    {
        status = xQueueSendFromISR(ICall_entities[dest].task->syncHandle, (char *) &msg_ptr, NULL);
    }
    else
    {
        status = xQueueSend(ICall_entities[dest].task->syncHandle, (char *) &msg_ptr, 0);
    }

    if (status != pdTRUE)
    {
        return status;
    }
#endif
    return (ICALL_ERRNO_SUCCESS);
}
#endif /* ICALL_LITE*/
