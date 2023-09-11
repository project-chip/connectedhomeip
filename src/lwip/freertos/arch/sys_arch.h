/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018-2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *    Description:
 *      LwIP sys_arch definitions for use with FreeRTOS.
 *
 */

#ifndef CHIP_LWIP_FREERTOS_ARCH_SYS_ARCH_H
#define CHIP_LWIP_FREERTOS_ARCH_SYS_ARCH_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define SYS_MBOX_NULL ((xQueueHandle) NULL)
#define SYS_SEM_NULL ((xSemaphoreHandle) NULL)
#define SYS_DEFAULT_THREAD_STACK_DEPTH configMINIMAL_STACK_SIZE
#define SYS_MESG_QUEUE_LENGTH ((UBaseType_t) 6)
#define SYS_POST_BLOCK_TIME_MS 10000

typedef QueueHandle_t sys_mbox_t;
typedef xSemaphoreHandle sys_mutex_t;
typedef xSemaphoreHandle sys_sem_t;
typedef TaskHandle_t sys_thread_t;
typedef UBaseType_t sys_prot_t;

#include "arch/sys_arch.h"
#include "lwip/opt.h"
#include "lwip/sys.h"

#define sys_sem_valid(sem) (((sem) != NULL) && (*(sem) != NULL))
#define sys_sem_set_invalid(sem)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        if ((sem) != NULL)                                                                                                         \
        {                                                                                                                          \
            *(sem) = NULL;                                                                                                         \
        }                                                                                                                          \
    } while (0)

#define sys_mutex_valid(mutex) (((mutex) != NULL) && (*(mutex) != NULL))
#define sys_mutex_set_invalid(mutex)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        if ((mutex) != NULL)                                                                                                       \
        {                                                                                                                          \
            *(mutex) = NULL;                                                                                                       \
        }                                                                                                                          \
    } while (0)

#define sys_mbox_valid(mbox) (((mbox) != NULL) && (*(mbox) != NULL))
#define sys_mbox_set_invalid(mbox)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        if ((mbox) != NULL)                                                                                                        \
        {                                                                                                                          \
            *(mbox) = NULL;                                                                                                        \
        }                                                                                                                          \
    } while (0)

#define sys_profile_interval_set_pbuf_highwatermark(...)

#endif /* CHIP_LWIP_FREERTOS_ARCH_SYS_ARCH_H */
