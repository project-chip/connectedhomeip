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
 *      LwIP sys_arch definitions for use with FreeRTOS.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "arch/sys_arch.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/stats.h"
#include "lwip/sys.h"

#ifndef LWIP_FREERTOS_USE_STATIC_TCPIP_TASK
#define LWIP_FREERTOS_USE_STATIC_TCPIP_TASK configSUPPORT_STATIC_ALLOCATION
#endif

#ifndef LWIP_FREERTOS_USE_STATIC_TCPIP_QUEUE
#define LWIP_FREERTOS_USE_STATIC_TCPIP_QUEUE configSUPPORT_STATIC_ALLOCATION
#endif

#if LWIP_FREERTOS_USE_STATIC_TCPIP_TASK
static StaticTask_t gTCPIPTask;
static StackType_t gTCPIPTaskStack[TCPIP_THREAD_STACKSIZE / sizeof(StackType_t)];
#endif

#if LWIP_FREERTOS_USE_STATIC_TCPIP_QUEUE
static StaticQueue_t gTCPIPMsgQueue;
static uint8_t gTCPIPMsgQueueStorage[SYS_MESG_QUEUE_LENGTH * sizeof(void *)];
#endif

static inline u32_t TicksToMS(TickType_t ticks)
{
    return (ticks * 1000) / configTICK_RATE_HZ;
}

void sys_init(void)
{
    // nothing to do.
}

err_t sys_sem_new(sys_sem_t * sem, u8_t count)
{
    *sem = xSemaphoreCreateBinary();
    if (*sem != NULL)
    {
        if (count != 0)
        {
            xSemaphoreGive(*sem);
        }
        SYS_STATS_INC_USED(sem);
        return ERR_OK;
    }
    else
    {
        SYS_STATS_INC(sem.err);
        return ERR_MEM;
    }
}

void sys_sem_free(sys_sem_t * sem)
{
    vSemaphoreDelete(*sem);
    SYS_STATS_DEC(sem);
}

void sys_sem_signal(sys_sem_t * sem)
{
    xSemaphoreGive(*sem);
}

u32_t sys_arch_sem_wait(sys_sem_t * sem, u32_t timeout)
{
    TickType_t timeoutTicks, startTime;
    BaseType_t res;

    if (timeout == 0)
        timeoutTicks = portMAX_DELAY;
    else
        timeoutTicks = pdMS_TO_TICKS(timeout);

    startTime = xTaskGetTickCount();

    do
    {
        res = xSemaphoreTake(*sem, timeoutTicks);
    } while (res != pdTRUE && timeout == 0);

    if (res == pdTRUE)
    {
        u32_t elapsedTime = TicksToMS(xTaskGetTickCount() - startTime);
        if (elapsedTime == 0)
            elapsedTime = 1;
        return elapsedTime;
    }
    else
        return SYS_ARCH_TIMEOUT;
}

err_t sys_mutex_new(sys_mutex_t * mutex)
{
    *mutex = xSemaphoreCreateMutex();
    if (*mutex != NULL)
    {
        xSemaphoreGive(*mutex);
        SYS_STATS_INC_USED(mutex);
        return ERR_OK;
    }
    else
    {
        SYS_STATS_INC(mutex.err);
        return ERR_MEM;
    }
}

void sys_mutex_free(sys_mutex_t * mutex)
{
    vSemaphoreDelete(*mutex);
    SYS_STATS_DEC(mutex);
}

void sys_mutex_lock(sys_mutex_t * mutex)
{
    xSemaphoreTake(*mutex, portMAX_DELAY);
}

void sys_mutex_unlock(sys_mutex_t * mutex)
{
    xSemaphoreGive(*mutex);
}

err_t sys_mbox_new(sys_mbox_t * mbox, int size)
{
    if (size != SYS_MESG_QUEUE_LENGTH)
    {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }

#if LWIP_FREERTOS_USE_STATIC_TCPIP_QUEUE
    *mbox = xQueueCreateStatic((UBaseType_t) size, (UBaseType_t) sizeof(void *), gTCPIPMsgQueueStorage, &gTCPIPMsgQueue);
#else
    *mbox = xQueueCreate((UBaseType_t) size, (UBaseType_t) sizeof(void *));
#endif
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

void sys_mbox_free(sys_mbox_t * mbox)
{
    vQueueDelete(*mbox);
}

void sys_mbox_post(sys_mbox_t * mbox, void * msg)
{
    BaseType_t res;
    res = xQueueSendToBack(*mbox, &msg, pdMS_TO_TICKS(SYS_POST_BLOCK_TIME_MS));
    LWIP_ASSERT("Error posting to LwIP mbox", res == pdTRUE);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t * mbox, void ** msg, u32_t timeout)
{
    TickType_t timeoutTicks, startTime;
    BaseType_t res;
    void * dummy;

    if (msg == NULL)
        msg = &dummy;

    if (timeout == 0)
        timeoutTicks = portMAX_DELAY;
    else
        timeoutTicks = pdMS_TO_TICKS(timeout);

    startTime = xTaskGetTickCount();

    do
    {
        res = xQueueReceive(*mbox, (void *) msg, timeoutTicks);
    } while (res != pdTRUE && timeout == 0);

    if (res == pdTRUE)
    {
        u32_t elapsedTime = TicksToMS(xTaskGetTickCount() - startTime);
        if (elapsedTime == 0)
            elapsedTime = 1;
        return elapsedTime;
    }
    else
    {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t * mbox, void ** msg)
{
    BaseType_t res;
    void * dummy;

    if (msg == NULL)
        msg = &dummy;

    res = xQueueReceive(*mbox, (void *) msg, 0);

    return (res == pdTRUE) ? 0 : SYS_MBOX_EMPTY;
}

err_t sys_mbox_trypost(sys_mbox_t * mbox, void * msg)
{
    BaseType_t res;

    res = xQueueSendToBack(*mbox, &msg, 0);

    if (res == pdTRUE)
        return ERR_OK;
    else
    {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

sys_thread_t sys_thread_new(const char * name, lwip_thread_fn thread, void * arg, int stacksize, int prio)
{
    TaskHandle_t taskH;
    const unsigned short stacksizeWords = (unsigned short) (stacksize / sizeof(StackType_t));

    if (strcmp(name, TCPIP_THREAD_NAME) != 0 || stacksize != TCPIP_THREAD_STACKSIZE)
        return NULL;

#if LWIP_FREERTOS_USE_STATIC_TCPIP_TASK
    taskH = xTaskCreateStatic(thread, name, stacksizeWords, arg, (UBaseType_t) prio, gTCPIPTaskStack, &gTCPIPTask);
#else  // LWIP_FREERTOS_USE_STATIC_TCPIP_TASK
    if (xTaskCreate(thread, name, stacksizeWords, arg, (UBaseType_t) prio, &taskH) != pdPASS)
        taskH = NULL;
#endif // LWIP_FREERTOS_USE_STATIC_TCPIP_TASK

    return taskH;
}

err_t sys_thread_finish(sys_thread_t t)
{
#if INCLUDE_vTaskDelete
    TaskHandle_t taskH = (TaskHandle_t) t;
    vTaskDelete(taskH);
#endif
    return ERR_OK;
}

u32_t sys_now(void)
{
    return TicksToMS(xTaskGetTickCount());
}

sys_prot_t sys_arch_protect(void)
{
    return taskENTER_CRITICAL_FROM_ISR();
}

void sys_arch_unprotect(sys_prot_t pval)
{
    taskEXIT_CRITICAL_FROM_ISR(pval);
}
