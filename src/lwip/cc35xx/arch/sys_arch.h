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
 *    Description:
 *      LwIP sys_arch definitions for use with FreeRTOS.
 *
 */

#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include "lwip/arch.h"
#include "lwip/opt.h"

/** This is returned by _fromisr() sys functions to tell the outermost function
 * that a higher priority task was woken and the scheduler needs to be invoked.
 */
#define ERR_NEED_SCHED 123

/* This port includes FreeRTOS headers in sys_arch.c only.
 *  FreeRTOS uses pointers as object types. We use wrapper structs instead of
 * void pointers directly to get a tiny bit of type safety.
 */

void sys_arch_msleep(u32_t delay_ms);
#define sys_msleep(ms) sys_arch_msleep(ms)

#if SYS_LIGHTWEIGHT_PROT
// typedef u32_t sys_prot_t;
#endif /* SYS_LIGHTWEIGHT_PROT */

#if !LWIP_COMPAT_MUTEX
struct _sys_mut
{
    void * mut;
};
typedef struct _sys_mut sys_mutex_t;
#define sys_mutex_valid_val(mutex) ((mutex).mut != NULL)
#define sys_mutex_valid(mutex) (((mutex) != NULL) && sys_mutex_valid_val(*(mutex)))
#define sys_mutex_set_invalid(mutex) ((mutex)->mut = NULL)
#endif /* !LWIP_COMPAT_MUTEX */

struct _sys_sem
{
    void * sem;
};
typedef struct _sys_sem sys_sem_t;
#define sys_sem_valid_val(sema) ((sema).sem != NULL)
#define sys_sem_valid(sema) (((sema) != NULL) && sys_sem_valid_val(*(sema)))
#define sys_sem_set_invalid(sema) ((sema)->sem = NULL)

struct _sys_mbox
{
    void * mbx;
};
typedef struct _sys_mbox sys_mbox_t;
#define sys_mbox_valid_val(mbox) ((mbox).mbx != NULL)
#define sys_mbox_valid(mbox) (((mbox) != NULL) && sys_mbox_valid_val(*(mbox)))
#define sys_mbox_set_invalid(mbox) ((mbox)->mbx = NULL)

struct _sys_thread
{
    void * thread_handle;
};
typedef struct _sys_thread sys_thread_t;

#if LWIP_NETCONN_SEM_PER_THREAD
sys_sem_t * sys_arch_netconn_sem_get(void);
void sys_arch_netconn_sem_alloc(void);
void sys_arch_netconn_sem_free(void);
#define LWIP_NETCONN_THREAD_SEM_GET() sys_arch_netconn_sem_get()
#define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_arch_netconn_sem_alloc()
#define LWIP_NETCONN_THREAD_SEM_FREE() sys_arch_netconn_sem_free()
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* LWIP_ARCH_SYS_ARCH_H */
