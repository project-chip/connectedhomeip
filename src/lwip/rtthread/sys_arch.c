/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018-2019 Google LLC.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/*
 *
 *    Description:
 *      LwIP sys_arch definitions for use with RT-Thread.
 *
 */

#include "arch/sys_arch.h"
#include <lwip/stats.h>

#include "arch/sys_arch.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"

void sys_init(void)
{
    // Nothing to do here for RT-Thread.
}

/*
 * Create a new semaphore
 *
 * @return the operation status, ERR_OK on OK; others on error
 */
err_t sys_sem_new(sys_sem_t * sem, u8_t count)
{
    *sem = rt_sem_create("lwip_sem", count, RT_IPC_FLAG_PRIO);
    if (*sem != RT_EOK)
    {
        SYS_STATS_INC_USED(sem);
        return ERR_OK;
    }
    else
    {
        SYS_STATS_INC(sem.err);
        return ERR_MEM;
    }
}

/*
 * Deallocate a semaphore
 */
void sys_sem_free(sys_sem_t * sem)
{
    rt_sem_delete(*sem);
    SYS_STATS_DEC(sem);
}

/*
 * Signal a semaphore
 */
void sys_sem_signal(sys_sem_t * sem)
{
    rt_sem_release(*sem);
}

/*
 * Block the thread while waiting for the semaphore to be signaled
 *
 * @return If the timeout argument is non-zero, it will return the number of milliseconds
 *         spent waiting for the semaphore to be signaled; If the semaphore isn't signaled
 *         within the specified time, it will return SYS_ARCH_TIMEOUT; If the thread doesn't
 *         wait for the semaphore, it will return zero
 */
u32_t sys_arch_sem_wait(sys_sem_t * sem, u32_t timeout)
{
    rt_tick_t start_time    = rt_tick_get();
    rt_tick_t timeout_ticks = RT_TICK_PER_SECOND * timeout / 1000;

    if (timeout == 0)
    {
        timeout_ticks = RT_WAITING_FOREVER;
    }
    else
    {
        /* convert msecond to os tick */
        if (timeout < (1000 / RT_TICK_PER_SECOND))
            t = 1;
        else
            t = timeout / (1000 / RT_TICK_PER_SECOND);
    }

    rt_err_t result = rt_sem_take(*sem, timeout_ticks);
    if (result == RT_EOK)
    {
        u32_t elapsed_time = (rt_tick_get() - start_time) * 1000 / RT_TICK_PER_SECOND;
        if (elapsed_time == 0)
        {
            elapsed_time = 1;
        }
        return elapsed_time;
    }
    else if (result == -RT_ETIMEOUT)
    {
        return SYS_ARCH_TIMEOUT;
    }
    else
    {
        return SYS_ARCH_TIMEOUT;
    }
}

#ifndef sys_sem_valid
/** Check if a semaphore is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_sem_valid(sys_sem_t * sem)
{
    return (int) (*sem);
}
#endif

#ifndef sys_sem_set_invalid
/** Set a semaphore invalid so that sys_sem_valid returns 0
 */
void sys_sem_set_invalid(sys_sem_t * sem)
{
    *sem = RT_NULL;
}
#endif

/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex
 */
err_t sys_mutex_new(sys_mutex_t * mutex)
{
    *mutex = rt_mutex_create("lwip_mutex", RT_IPC_FLAG_PRIO);
    if (*mutex != RT_NULL)
    {
        SYS_STATS_INC_USED(mutex);
        return ERR_OK;
    }
    else
    {
        SYS_STATS_INC(mutex.err);
        return ERR_MEM;
    }
}

/** Delete a semaphore
 * @param mutex the mutex to delete
 */
void sys_mutex_free(sys_mutex_t * mutex)
{
    rt_mutex_delete(*mutex);
    SYS_STATS_DEC(mutex);
}

/** Lock a mutex
 * @param mutex the mutex to lock
 */
void sys_mutex_lock(sys_mutex_t * mutex)
{
    rt_mutex_take(*mutex, RT_WAITING_FOREVER);
}

/** Unlock a mutex
 * @param mutex the mutex to unlock
 */
void sys_mutex_unlock(sys_mutex_t * mutex)
{
    rt_mutex_release(*mutex);
}

#ifndef sys_mutex_valid 
/** Check if a mutex is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mutex_valid(sys_mutex_t * mutex)
{
    return (int) (*mutex);
}
#endif

#ifndef sys_mutex_set_invalid
/** Set a mutex invalid so that sys_mutex_valid returns 0
 */
void sys_mutex_set_invalid(sys_mutex_t * mutex)
{
    *mutex = RT_NULL;
}
#endif

/*
 * Create an empty mailbox for maximum "size" elements
 *
 * @return the operation status, ERR_OK on OK; others on error
 */
err_t sys_mbox_new(sys_mbox_t * mbox, int size)
{
    *mbox = rt_mb_create("lwip_mbox", size, RT_IPC_FLAG_FIFO);
    if (*mbox != NULL)
    {
        SYS_STATS_INC_USED(mbox);
        return ERR_OK;
    }
    else
    {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

/*
 * Deallocate a mailbox
 */
void sys_mbox_free(sys_mbox_t * mbox)
{
    rt_mb_delete(*mbox);
}

/** Post a message to an mbox - may not fail
 * -> blocks if full, only used from tasks not from ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
void sys_mbox_post(sys_mbox_t * mbox, void * msg)
{
    rt_mb_send_wait(*mbox, (rt_uint32_t) msg, RT_WAITING_FOREVER);
}

/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return time (in milliseconds) waited for a message, may be 0 if not waited
           or SYS_ARCH_TIMEOUT on timeout
 *         The returned time has to be accurate to prevent timer jitter!
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t * mbox, void ** msg, u32_t timeout)
{
    rt_tick_t start_time    = rt_tick_get();
    rt_tick_t timeout_ticks = RT_TICK_PER_SECOND * timeout / 1000;

    if (msg == NULL)
    {
        msg = (void **) rt_malloc(sizeof(void *));
    }

    rt_err_t result = rt_mb_recv(*mbox, (rt_ubase_t *) msg, timeout_ticks);
    if (result == RT_EOK)
    {
        u32_t elapsed_time = (rt_tick_get() - start_time) * 1000 / RT_TICK_PER_SECOND;
        if (elapsed_time == 0)
        {
            elapsed_time = 1;
        }
        return elapsed_time;
    }
    else if (result == -RT_ETIMEOUT)
    {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }
    else
    {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }
}

/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t * mbox, void ** msg)
{
    rt_err_t result = rt_mb_recv(*mbox, (rt_ubase_t *) msg, 0);
    return (result == RT_EOK) ? 0 : SYS_MBOX_EMPTY;
}

/*
 * Try to post the "msg" to the mailbox
 *
 * @return return ERR_OK if the "msg" is posted, ERR_MEM if the mailbox is full
 */
err_t sys_mbox_trypost(sys_mbox_t * mbox, void * msg)
{
    rt_err_t result = rt_mb_send(*mbox, (rt_uint32_t) msg);
    if (result == RT_EOK)
    {
        return ERR_OK;
    }
    else
    {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

#ifndef sys_mbox_valid
/** Check if an mbox is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mbox_valid(sys_mbox_t * mbox)
{
    return (int) (*mbox);
}
#endif

#ifndef sys_mbox_set_invalid
/** Set an mbox invalid so that sys_mbox_valid returns 0
 */
void sys_mbox_set_invalid(sys_mbox_t * mbox)
{
    *mbox = RT_NULL;
}
#endif

/*
 * Start a new thread named "name" with priority "prio" that will begin
 * its execution in the function "thread()". The "arg" argument will be
 * passed as an argument to the thread() function
 */
sys_thread_t sys_thread_new(const char * name, lwip_thread_fn thread, void * arg, int stacksize, int prio)
{
    rt_thread_t thread_id;
    thread_id = rt_thread_create(name, thread, arg, stacksize, prio, 20);
    if (thread_id != RT_NULL)
    {
        rt_thread_startup(thread_id);
    }
    return thread_id;
}

err_t sys_thread_finish(sys_thread_t t)
{
    rt_thread_t thread_id = (rt_thread_t) t;
    rt_thread_delete(thread_id);
    return ERR_OK;
}

u32_t sys_now(void)
{
    return rt_tick_get_millisecond();
}

sys_prot_t sys_arch_protect(void)
{
    sys_prot_t state;
    rt_base_t level;
    level = rt_hw_interrupt_disable();
    state = level;
    return state;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    rt_hw_interrupt_enable(pval);
}