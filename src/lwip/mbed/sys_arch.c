/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
 *    Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 *    All rights reserved.
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
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Wed Apr 17 16:05:29 EDT 2002 (James Roth)
 *
 *  - Fixed an unlikely sys_thread_new() race condition.
 *
 *  - Made current_thread() work with threads which where
 *    not created with sys_thread_new().  This includes
 *    the main thread and threads made with pthread_create().
 *
 *  - Catch overflows where more than SYS_MBOX_SIZE messages
 *    are waiting to be read.  The sys_mbox_post() routine
 *    will block until there is more room instead of just
 *    leaking messages.
 */
#include "lwip/debug.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>


#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"

#define UMAX(a, b) ((a) > (b) ? (a) : (b))

static struct timeval starttime;

#define NO_SYS 1

#if !NO_SYS

static struct sys_thread * threads   = NULL;
static pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;

struct sys_mbox_msg
{
    struct sys_mbox_msg * next;
    void * msg;
};

#define SYS_MBOX_SIZE 128

struct sys_mbox
{
    int first, last;
    void * msgs[SYS_MBOX_SIZE];
    struct sys_sem * not_empty;
    struct sys_sem * not_full;
    struct sys_sem * mutex;
    int wait_send;
};

struct sys_sem
{
    unsigned int c;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};

struct sys_thread
{
    struct sys_thread * next;
    pthread_t pthread;
};

#if SYS_LIGHTWEIGHT_PROT
static pthread_mutex_t lwprot_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t lwprot_thread      = (pthread_t) 0xDEAD;
static int lwprot_count             = 0;
#endif /* SYS_LIGHTWEIGHT_PROT */

static struct sys_sem * sys_sem_new_internal(u8_t count);
static void sys_sem_free_internal(struct sys_sem * sem);

static u32_t cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex, u32_t timeout);

/*-----------------------------------------------------------------------------------*/
static struct sys_thread * introduce_thread(pthread_t id)
{
    struct sys_thread * thread;

    thread = (struct sys_thread *) CHIPPlatformMemoryAlloc(sizeof(struct sys_thread));

    if (thread != NULL)
    {
        pthread_mutex_lock(&threads_mutex);
        thread->next    = threads;
        thread->pthread = id;
        threads         = thread;
        pthread_mutex_unlock(&threads_mutex);
    }

    return thread;
}
/*-----------------------------------------------------------------------------------*/
static void finish_thread(struct sys_thread * thread)
{
    struct sys_thread * cursor;
    struct sys_thread * previous;

    if (thread != NULL)
    {
        pthread_mutex_lock(&threads_mutex);

        previous = NULL;
        cursor   = threads;
        while (cursor != NULL)
        {
            if (cursor == thread)
            {
                if (previous != NULL)
                {
                    previous->next = cursor->next;
                }
                else
                {
                    threads = cursor->next;
                }
                cursor->next    = NULL;
                cursor->pthread = 0;
                break;
            }
            previous = cursor;
            cursor   = cursor->next;
        }

        pthread_mutex_unlock(&threads_mutex);
        CHIPPlatformMemoryFree(thread);
    }
}
/*-----------------------------------------------------------------------------------*/
sys_thread_t sys_thread_new(const char * name, lwip_thread_fn function, void * arg, int stacksize, int prio)
{
    int code;
    pthread_t tmp;
    struct sys_thread * st = NULL;
    LWIP_UNUSED_ARG(name);
    LWIP_UNUSED_ARG(stacksize);
    LWIP_UNUSED_ARG(prio);

    code = pthread_create(&tmp, NULL, (void * (*) (void *) ) function, arg);

    if (0 == code)
    {
        st = introduce_thread(tmp);
    }

    if (NULL == st)
    {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_thread_new: pthread_create %d, st = 0x%lx", code, (unsigned long) st));
        abort();
    }
    return st;
}
/*-----------------------------------------------------------------------------------*/
err_t sys_thread_finish(sys_thread_t t)
{
    int code;

    code = pthread_join(t->pthread, NULL);
    if (0 == code)
    {
        finish_thread(t);
        return ERR_OK;
    }

    return ERR_VAL;
}
/*-----------------------------------------------------------------------------------*/
err_t sys_mbox_new_extra(void * pool, struct sys_mbox ** mb, int size)
{
    return sys_mbox_new(mb, size);
}

err_t sys_mbox_new(struct sys_mbox ** mb, int size)
{
    struct sys_mbox * mbox;
    LWIP_UNUSED_ARG(size);

    mbox = (struct sys_mbox *) CHIPPlatformMemoryAlloc(sizeof(struct sys_mbox));
    if (mbox == NULL)
    {
        return ERR_MEM;
    }
    mbox->first = mbox->last = 0;
    mbox->not_empty          = sys_sem_new_internal(0);
    mbox->not_full           = sys_sem_new_internal(0);
    mbox->mutex              = sys_sem_new_internal(1);
    mbox->wait_send          = 0;

    SYS_STATS_INC_USED(mbox);
    *mb = mbox;
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void sys_mbox_free(struct sys_mbox ** mb)
{
    if ((mb != NULL) && (*mb != SYS_MBOX_NULL))
    {
        struct sys_mbox * mbox = *mb;
        SYS_STATS_DEC(mbox.used);
        sys_arch_sem_wait(&mbox->mutex, 0);

        sys_sem_free_internal(mbox->not_empty);
        sys_sem_free_internal(mbox->not_full);
        sys_sem_free_internal(mbox->mutex);
        mbox->not_empty = mbox->not_full = mbox->mutex = NULL;
        /*  LWIP_DEBUGF("sys_mbox_free: mbox 0x%lx\n", mbox); */
        CHIPPlatformMemoryFree(mbox);
    }
}
/*-----------------------------------------------------------------------------------*/
err_t sys_mbox_trypost(struct sys_mbox ** mb, void * msg)
{
    u8_t first;
    struct sys_mbox * mbox;
    LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
    mbox = *mb;

    sys_arch_sem_wait(&mbox->mutex, 0);

    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_trypost: mbox %p msg %p\n", (void *) mbox, (void *) msg));

    if ((mbox->last + 1) >= (mbox->first + SYS_MBOX_SIZE))
    {
        sys_sem_signal(&mbox->mutex);
        return ERR_MEM;
    }

    mbox->msgs[mbox->last % SYS_MBOX_SIZE] = msg;

    if (mbox->last == mbox->first)
    {
        first = 1;
    }
    else
    {
        first = 0;
    }

    mbox->last++;

    if (first)
    {
        sys_sem_signal(&mbox->not_empty);
    }

    sys_sem_signal(&mbox->mutex);

    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void sys_mbox_post(struct sys_mbox ** mb, void * msg)
{
    u8_t first;
    struct sys_mbox * mbox;
    LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
    mbox = *mb;

    sys_arch_sem_wait(&mbox->mutex, 0);

    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_post: mbox %p msg %p\n", (void *) mbox, (void *) msg));

    while ((mbox->last + 1) >= (mbox->first + SYS_MBOX_SIZE))
    {
        mbox->wait_send++;
        sys_sem_signal(&mbox->mutex);
        sys_arch_sem_wait(&mbox->not_full, 0);
        sys_arch_sem_wait(&mbox->mutex, 0);
        mbox->wait_send--;
    }

    mbox->msgs[mbox->last % SYS_MBOX_SIZE] = msg;

    if (mbox->last == mbox->first)
    {
        first = 1;
    }
    else
    {
        first = 0;
    }

    mbox->last++;

    if (first)
    {
        sys_sem_signal(&mbox->not_empty);
    }

    sys_sem_signal(&mbox->mutex);
}
/*-----------------------------------------------------------------------------------*/
u32_t sys_arch_mbox_tryfetch(struct sys_mbox ** mb, void ** msg)
{
    struct sys_mbox * mbox;
    LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
    mbox = *mb;

    sys_arch_sem_wait(&mbox->mutex, 0);

    if (mbox->first == mbox->last)
    {
        sys_sem_signal(&mbox->mutex);
        return SYS_MBOX_EMPTY;
    }

    if (msg != NULL)
    {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_tryfetch: mbox %p msg %p\n", (void *) mbox, *msg));
        *msg = mbox->msgs[mbox->first % SYS_MBOX_SIZE];
    }
    else
    {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_tryfetch: mbox %p, null msg\n", (void *) mbox));
    }

    mbox->first++;

    if (mbox->wait_send)
    {
        sys_sem_signal(&mbox->not_full);
    }

    sys_sem_signal(&mbox->mutex);

    return 0;
}
/*-----------------------------------------------------------------------------------*/
u32_t sys_arch_mbox_fetch(struct sys_mbox ** mb, void ** msg, u32_t timeout)
{
    u32_t time_needed = 0;
    struct sys_mbox * mbox;
    LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
    mbox = *mb;

    /* The mutex lock is quick so we don't bother with the timeout
       stuff here. */
    sys_arch_sem_wait(&mbox->mutex, 0);

    while (mbox->first == mbox->last)
    {
        sys_sem_signal(&mbox->mutex);

        /* We block while waiting for a mail to arrive in the mailbox. We
           must be prepared to timeout. */
        if (timeout != 0)
        {
            time_needed = sys_arch_sem_wait(&mbox->not_empty, timeout);

            if (time_needed == SYS_ARCH_TIMEOUT)
            {
                return SYS_ARCH_TIMEOUT;
            }
        }
        else
        {
            sys_arch_sem_wait(&mbox->not_empty, 0);
        }

        sys_arch_sem_wait(&mbox->mutex, 0);
    }

    if (msg != NULL)
    {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p msg %p\n", (void *) mbox, *msg));
        *msg = mbox->msgs[mbox->first % SYS_MBOX_SIZE];
    }
    else
    {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p, null msg\n", (void *) mbox));
    }

    mbox->first++;

    if (mbox->wait_send)
    {
        sys_sem_signal(&mbox->not_full);
    }

    sys_sem_signal(&mbox->mutex);

    return time_needed;
}
/*-----------------------------------------------------------------------------------*/
static struct sys_sem * sys_sem_new_internal(u8_t count)
{
    struct sys_sem * sem;

    sem = (struct sys_sem *) CHIPPlatformMemoryAlloc(sizeof(struct sys_sem));
    if (sem != NULL)
    {
        sem->c = count;
        pthread_cond_init(&(sem->cond), NULL);
        pthread_mutex_init(&(sem->mutex), NULL);
    }
    return sem;
}
/*-----------------------------------------------------------------------------------*/
err_t sys_sem_new(struct sys_sem ** sem, u8_t count)
{
    SYS_STATS_INC_USED(sem);
    *sem = sys_sem_new_internal(count);
    if (*sem == NULL)
    {
        return ERR_MEM;
    }
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static u32_t cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex, u32_t timeout)
{
    time_t tdiff;
    time_t sec, usec;
    struct timeval rtime1, rtime2;
    struct timespec ts;
    int retval;

    if (timeout > 0)
    {
        /* Get a timestamp and add the timeout value. */
        gettimeofday(&rtime1, NULL);
        sec  = rtime1.tv_sec;
        usec = rtime1.tv_usec;
        usec += timeout % 1000 * 1000;
        sec += (int) (timeout / 1000) + (int) (usec / 1000000);
        usec       = usec % 1000000;
        ts.tv_nsec = usec * 1000;
        ts.tv_sec  = sec;

        retval = pthread_cond_timedwait(cond, mutex, &ts);

        if (retval == ETIMEDOUT)
        {
            return SYS_ARCH_TIMEOUT;
        }

        /* Calculate for how long we waited for the cond. */
        gettimeofday(&rtime2, NULL);
        tdiff = (rtime2.tv_sec - rtime1.tv_sec) * 1000 + (rtime2.tv_usec - rtime1.tv_usec) / 1000;

        if (tdiff <= 0)
        {
            return 0;
        }

        return (u32_t) tdiff;
    }

    pthread_cond_wait(cond, mutex);

    return 0;
}
/*-----------------------------------------------------------------------------------*/
u32_t sys_arch_sem_wait(struct sys_sem ** s, u32_t timeout)
{
    u32_t time_needed = 0;
    struct sys_sem * sem;
    LWIP_ASSERT("invalid sem", (s != NULL) && (*s != NULL));
    sem = *s;

    pthread_mutex_lock(&(sem->mutex));
    while (sem->c <= 0)
    {
        if (timeout > 0)
        {
            time_needed = cond_wait(&(sem->cond), &(sem->mutex), timeout);

            if (time_needed == SYS_ARCH_TIMEOUT)
            {
                pthread_mutex_unlock(&(sem->mutex));
                return SYS_ARCH_TIMEOUT;
            }
            /*      pthread_mutex_unlock(&(sem->mutex));
                    return time_needed; */
        }
        else
        {
            cond_wait(&(sem->cond), &(sem->mutex), 0);
        }
    }
    sem->c--;
    pthread_mutex_unlock(&(sem->mutex));
    return time_needed;
}
/*-----------------------------------------------------------------------------------*/
void sys_sem_signal(struct sys_sem ** s)
{
    struct sys_sem * sem;
    LWIP_ASSERT("invalid sem", (s != NULL) && (*s != NULL));
    sem = *s;

    pthread_mutex_lock(&(sem->mutex));
    sem->c++;

    if (sem->c > 1)
    {
        sem->c = 1;
    }

    pthread_cond_broadcast(&(sem->cond));
    pthread_mutex_unlock(&(sem->mutex));
}
/*-----------------------------------------------------------------------------------*/
static void sys_sem_free_internal(struct sys_sem * sem)
{
    pthread_cond_destroy(&(sem->cond));
    pthread_mutex_destroy(&(sem->mutex));
    CHIPPlatformMemoryFree(sem);
}
/*-----------------------------------------------------------------------------------*/
void sys_sem_free(struct sys_sem ** sem)
{
    if ((sem != NULL) && (*sem != SYS_SEM_NULL))
    {
        SYS_STATS_DEC(sem.used);
        sys_sem_free_internal(*sem);
    }
}

#else 

u32_t sys_arch_sem_wait(sys_sem_t * sem, u32_t timeout)
{
    return 0;
}

void sys_sem_signal(struct sys_sem ** s)
{

}

err_t sys_mbox_trypost(struct sys_mbox ** mb, void * msg)
{
    return ERR_OK;
}

void sys_mbox_post(struct sys_mbox ** mb, void * msg)
{
    
}

#endif /* !NO_SYS */

/*-----------------------------------------------------------------------------------*/
u32_t sys_now(void)
{
    struct timeval tv;
    u32_t msec;
    gettimeofday(&tv, NULL);

    msec = (u32_t)((tv.tv_sec - starttime.tv_sec) * 1000 + (tv.tv_usec - starttime.tv_usec) / 1000);

    return msec;
}
/*-----------------------------------------------------------------------------------*/
void sys_init(void)
{
    gettimeofday(&starttime, NULL);
}
/*-----------------------------------------------------------------------------------*/
#if SYS_LIGHTWEIGHT_PROT
/** sys_prot_t sys_arch_protect(void)

This optional function does a "fast" critical region protection and returns
the previous protection level. This function is only called during very short
critical regions. An embedded system which supports ISR-based drivers might
want to implement this function by disabling interrupts. Task-based systems
might want to implement this by using a mutex or disabling tasking. This
function should support recursive calls from the same task or interrupt. In
other words, sys_arch_protect() could be called while already protected. In
that case the return value indicates that it is already protected.

sys_arch_protect() is only required if your port is supporting an operating
system.
*/
sys_prot_t sys_arch_protect(void)
{
    /* Note that for the UNIX port, we are using a lightweight mutex, and our
     * own counter (which is locked by the mutex). The return code is not actually
     * used. */
    if (lwprot_thread != pthread_self())
    {
        /* We are locking the mutex where it has not been locked before *
         * or is being locked by another thread */
        pthread_mutex_lock(&lwprot_mutex);
        lwprot_thread = pthread_self();
        lwprot_count  = 1;
    }
    else
        /* It is already locked by THIS thread */
        lwprot_count++;
    return 0;
}
/*-----------------------------------------------------------------------------------*/

/** void sys_arch_unprotect(sys_prot_t pval)

This optional function does a "fast" set of critical region protection to the
value specified by pval. See the documentation for sys_arch_protect() for
more information. This function is only required if your port is supporting
an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
    LWIP_UNUSED_ARG(pval);
    if (lwprot_thread == pthread_self())
    {
        if (--lwprot_count == 0)
        {
            lwprot_thread = (pthread_t) 0xDEAD;
            pthread_mutex_unlock(&lwprot_mutex);
        }
    }
}
#endif /* SYS_LIGHTWEIGHT_PROT */

/*-----------------------------------------------------------------------------------*/

#ifndef MAX_JIFFY_OFFSET
#define MAX_JIFFY_OFFSET ((~0U >> 1) - 1)
#endif

#ifndef HZ
#define HZ 100
#endif

u32_t sys_jiffies(void)
{
    struct timeval tv;
    unsigned long sec;
    long usec;

    gettimeofday(&tv, NULL);
    sec  = tv.tv_sec - starttime.tv_sec;
    usec = tv.tv_usec;

    if (sec >= (MAX_JIFFY_OFFSET / HZ))
        return MAX_JIFFY_OFFSET;
    usec += 1000000L / HZ - 1;
    usec /= 1000000L / HZ;
    return HZ * sec + usec;
}

#if PPP_DEBUG

#include <stdarg.h>

void ppp_trace(int level, const char * format, ...)
{
    va_list args;

    (void) level;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
#endif

#ifdef LWIP_DEBUG

unsigned char gLwIP_DebugFlags = 0;

#endif
