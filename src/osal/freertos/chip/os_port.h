/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC
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

#ifndef CHIP_OS_PORT_H
#define CHIP_OS_PORT_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHIP_OS_TIME_NO_WAIT 0
#define CHIP_OS_TIME_FOREVER portMAX_DELAY
#define CHIP_OS_TICKS_PER_SEC configTICK_RATE_HZ

#define CHIP_OS_PRIORITY_MIN 1
#define CHIP_OS_PRIORITY_MAX configMAX_PRIORITIES
#define CHIP_OS_PRIORITY_APP (CHIP_OS_PRIORITY_MIN + 1)

typedef BaseType_t chip_os_base_t;
typedef StackType_t chip_os_stack_t;

typedef TickType_t chip_os_time_t;
typedef int32_t chip_os_stime_t;

struct chip_os_mutex
{
    SemaphoreHandle_t handle;
};

struct chip_os_sem
{
    SemaphoreHandle_t handle;
};

struct chip_os_queue
{
    QueueHandle_t handle;
};

struct chip_os_timer
{
    TimerHandle_t handle;
    chip_os_timer_fn * func;
    void * arg;
};

struct chip_os_task
{
    TaskHandle_t handle;
    chip_os_task_func_t func;
    void * arg;
};

static inline bool chip_os_os_started(void)
{
    return xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED;
}

static inline uint16_t chip_os_sem_get_count(struct chip_os_sem * sem)
{
    assert(sem);
    assert(sem->handle);
    return uxSemaphoreGetCount(sem->handle);
}

static inline chip_os_error_t chip_os_timer_stop(struct chip_os_timer * tm)
{
    assert(tm);
    assert(tm->handle);
    return xTimerStop(tm->handle, portMAX_DELAY);
}

static inline bool chip_os_timer_is_active(struct chip_os_timer * tm)
{
    assert(tm);
    assert(tm->handle);
    return xTimerIsTimerActive(tm->handle) == pdTRUE;
}

static inline chip_os_error_t chip_os_queue_init(struct chip_os_queue * queue, size_t msg_size, size_t max_msgs)
{
    queue->handle = xQueueCreate(max_msgs, msg_size);
    return CHIP_OS_OK;
}

static inline int chip_os_queue_inited(const struct chip_os_queue * queue)
{
    return (queue->handle != NULL);
}

chip_os_error_t chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t ticks);

static inline chip_os_time_t chip_os_time_ms_to_ticks(chip_os_time_t ms)
{
    return (ms * CHIP_OS_TICKS_PER_SEC) / 1000;
}

static inline chip_os_time_t chip_os_time_ticks_to_ms(chip_os_time_t ticks)
{
    return (ticks * 1000) / CHIP_OS_TICKS_PER_SEC;
}

static inline chip_os_time_t chip_os_time_get(void)
{
    return xTaskGetTickCountFromISR();
}

static inline chip_os_time_t chip_os_time_get_ms(void)
{
    return chip_os_time_ticks_to_ms(chip_os_time_get());
}

static inline chip_os_error_t chip_os_timer_start_ms(struct chip_os_timer * timer, chip_os_time_t duration)
{
    chip_os_time_t ticks = chip_os_time_ms_to_ticks(duration);
    return chip_os_timer_start(timer, ticks);
}

static inline chip_os_time_t chip_os_timer_get_ticks(struct chip_os_timer * tm)
{
    assert(tm);
    assert(tm->handle);
    return xTimerGetExpiryTime(tm->handle);
}

static inline void * chip_os_timer_arg_get(struct chip_os_timer * timer)
{
    assert(timer);
    return timer->arg;
}

static inline void chip_os_timer_arg_set(struct chip_os_timer * timer, void * arg)
{
    assert(timer);
    timer->arg = arg;
}

static inline void chip_os_task_yield(void)
{
    taskYIELD();
}

static inline void chip_os_task_sleep(chip_os_time_t ticks)
{
    vTaskDelay(ticks);
}

static inline void chip_os_task_sleep_ms(chip_os_time_t ms)
{
    chip_os_task_sleep(chip_os_time_ms_to_ticks(ms));
}

static inline void * chip_os_get_current_task_id(void)
{
    return xTaskGetCurrentTaskHandle();
}

static inline void chip_os_sched_start(void)
{
    vTaskStartScheduler();
}

#ifdef __cplusplus
}
#endif

#endif /* CHIP_OS_PORT_H */
