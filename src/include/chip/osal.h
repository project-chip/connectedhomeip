/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CHIP_OSAL_H
#define CHIP_OSAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void chip_os_timer_fn(void * arg);
typedef void chip_os_signal_fn(void * arg);
typedef void * (*chip_os_task_func_t)(void *);

enum chip_os_error
{
    CHIP_OS_OK              = 0,
    CHIP_OS_ENOMEM          = 1,
    CHIP_OS_EINVAL          = 2,
    CHIP_OS_INVALID_PARAM   = 3,
    CHIP_OS_MEM_NOT_ALIGNED = 4,
    CHIP_OS_BAD_MUTEX       = 5,
    CHIP_OS_TIMEOUT         = 6,
    CHIP_OS_ERR_IN_ISR      = 7,
    CHIP_OS_ERR_PRIV        = 8,
    CHIP_OS_OS_NOT_STARTED  = 9,
    CHIP_OS_ENOENT          = 10,
    CHIP_OS_EBUSY           = 11,
    CHIP_OS_ERROR           = 12,
};

typedef enum chip_os_error chip_os_error_t;

/* Include OS-specific definitions */
#include "chip/os_port.h"

/*
 * Mutexes
 */

/**
 * @brief Initialize a mutex.
 *
 * This routine initializes a mutex object, prior to its first use.
 *
 * Upon completion, the mutex is available and does not have an owner.
 *
 * @param mutex Address of the mutex.
 *
 * @return N/A
 */
chip_os_error_t chip_os_mutex_init(struct chip_os_mutex * mutex);

/**
 * @brief Lock a mutex.
 *
 * This routine locks @a mutex. If the mutex is locked by another task,
 * the calling task waits until the mutex becomes available or until
 * a timeout occurs.
 *
 * A task is permitted to lock a mutex it has already locked. The operation
 * completes immediately and the lock count is increased by 1.
 *
 * @param mutex Address of the mutex.
 * @param timeout Waiting period to lock the mutex (in milliseconds),
 *                or one of the special values CHIP_OS_TIME_NO_WAIT and CHIP_OS_TIME_FOREVER.
 *
 * @retval CHIP_OS_OK Mutex locked.
 * @retval CHIP_OS_EBUSY Returned without waiting.
 * @retval CHIP_OS_TIMEOUT Waiting period timed out.
 */
chip_os_error_t chip_os_mutex_take(struct chip_os_mutex * mutex, chip_os_time_t timeout);

/**
 * @brief Unlock a mutex.
 *
 * This routine unlocks @a mutex. The mutex must already be locked by the
 * calling task.
 *
 * The mutex cannot be claimed by another task until it has been unlocked by
 * the calling task as many times as it was previously locked by that
 * task.
 *
 * @param mutex Address of the mutex.
 *
 * @return N/A
 */
chip_os_error_t chip_os_mutex_give(struct chip_os_mutex * mutex);

/*
 * Semaphores
 */

/**
 * @brief Initialize a semaphore.
 *
 * This routine initializes a semaphore object, prior to its first use.
 *
 * @param sem Address of the semaphore.
 * @param tokens Initial semaphore count.
 *
 * @return N/A
 */
chip_os_error_t chip_os_sem_init(struct chip_os_sem * sem, uint16_t tokens);

/**
 * @brief Take a semaphore.
 *
 * This routine takes @a sem.
 *
 * @note Can be called by ISRs, but @a timeout must be set to CHIP_OS_TIME_NO_WAIT.
 *
 * @param sem Address of the semaphore.
 * @param timeout Waiting period to take the semaphore (in milliseconds),
 *                or one of the special values CHIP_OS_TIME_NO_WAIT and CHIP_OS_TIME_FOREVER.
 *
 * @retval CHIP_OS_OK Semaphore taken.
 * @retval CHIP_OS_EBUSY Returned without waiting.
 * @retval CHIP_OS_TIMEOUT Waiting period timed out.
 */
chip_os_error_t chip_os_sem_take(struct chip_os_sem * sem, chip_os_time_t timeout);

/**
 * @brief Give a semaphore.
 *
 * This routine gives @a sem, unless the semaphore is already at its maximum
 * permitted count.
 *
 * @note Can be called by ISRs.
 *
 * @param sem Address of the semaphore.
 *
 * @return N/A
 */
chip_os_error_t chip_os_sem_give(struct chip_os_sem * sem);

/*
 * Message queue
 */

/**
 * @brief Initialize a message queue.
 *
 * This routine initializes a message queue object, prior to its first use.
 *
 * Message queues deliver complete copies of fixed size messages from a producer
 * to a consumer in a thread-safe manner.
 *
 * @param msgq Address of the message queue.
 * @param msg_size Message size (in bytes).
 * @param max_msgs Maximum number of messages that can be queued.
 *
 * @return N/A
 */
chip_os_error_t chip_os_queue_init(struct chip_os_queue * msgq, size_t msg_size, size_t max_msgs);

/**
 * @brief Receive a message from a message queue.
 *
 * This routine receives a message from message queue @a q in a "first in,
 * first out" manner.
 *
 * @note Can be called by ISRs, but @a timeout must be set to CHIP_OS_TIME_NO_WAIT.
 *
 * @param msgq Address of the message queue.
 * @param data Address of area to hold the received message.
 * @param timeout Waiting period to receive the message (in milliseconds),
 *                or one of the special values CHIP_OS_TIME_NO_WAIT and CHIP_OS_TIME_FOREVER.
 *
 * @retval CHIP_OS_OK Message received.
 * @retval CHIP_OS_EBUSY Returned without waiting.
 * @retval CHIP_OS_TIMEOUT Waiting period timed out.
 */
chip_os_error_t chip_os_queue_get(struct chip_os_queue * msgq, void * data, chip_os_time_t timeout);

/**
 * @brief Send a message to a message queue.
 *
 * This routine sends a message to message queue @a q.
 *
 * @note Can be called by ISRs.
 *
 * @param msgq Address of the message queue.
 * @param data Pointer to the message.
 *
 * @retval CHIP_OS_OK Message sent.
 * @retval CHIP_OS_EBUSY Returned without waiting or queue purged.
 * @retval CHIP_OS_TIMEOUT Waiting period timed out.
 */
chip_os_error_t chip_os_queue_put(struct chip_os_queue * msgq, void * data);

/**
 * @brief Returns whether the given message queue is initialized and valid.
 *
 * @param msgq Address of the message queue.
 *
 * @retval CHIP_OS_OK Queue is valid.
 * @retval CHIP_OS_EINVAL Queue is not valid.
 */
int chip_os_queue_inited(const struct chip_os_queue * msgq);

bool chip_os_queue_is_empty(struct chip_os_queue * msgq);

void chip_os_queue_set_signal_cb(struct chip_os_queue * msgq, chip_os_signal_fn signal_cb, void * data);

/*
 * Timer
 */

/**
 * @brief Initialize a timer.
 *
 * This routine initializes a timer, prior to its first use.
 *
 * @param timer     Address of timer.
 * @param cb        Callback function to invoke once timer expires.
 * @param arg       Argument to pass to timer callback function.
 *
 * @retval CHIP_OS_OK Timer was created.
 * @retval CHIP_OS_ERROR Timer creation failed.
 */
chip_os_error_t chip_os_timer_init(struct chip_os_timer * timer, chip_os_timer_fn cb, void * arg);

/**
 * @brief Start a timer.
 *
 * This routine starts a timer, and resets its status to zero. The timer
 * begins counting down using the specified duration and period values.
 *
 * Attempting to start a timer that is already running is permitted.
 * The timer's status is reset to zero and the timer begins counting down
 * using the new duration and period values.
 *
 * @param timer     Address of timer.
 * @param duration  Initial timer duration [ticks].
 *
 * @retval CHIP_OS_OK Message received.
 * @retval CHIP_OS_EBUSY Returned without waiting.
 */
chip_os_error_t chip_os_timer_start_ms(struct chip_os_timer * timer, chip_os_time_t duration);

/**
 * @brief Start a timer.
 *
 * This routine starts a timer, and resets its status to zero. The timer
 * begins counting down using the specified duration and period values.
 *
 * Attempting to start a timer that is already running is permitted.
 * The timer's status is reset to zero and the timer begins counting down
 * using the new duration and period values.
 *
 * @param timer     Address of timer.
 * @param ticks     Initial timer duration [CPU ticks].
 *
 * @retval CHIP_OS_OK Message received.
 * @retval CHIP_OS_EBUSY Returned without waiting.
 */
chip_os_error_t chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t ticks);

/**
 * @brief Stop a timer.
 *
 * This routine stops a running timer prematurely. The timer's stop function,
 * if one exists, is invoked by the caller.
 *
 * Attempting to stop a timer that is not running is permitted, but has no
 * effect on the timer.
 *
 * @note Can be called by ISRs.  The stop handler has to be callable from ISRs
 * if @a chip_os_timer_stop is to be called from ISRs.
 *
 * @param timer     Address of timer.
 *
 * @return N/A
 */
chip_os_error_t chip_os_timer_stop(struct chip_os_timer * timer);

/**
 * @brief Check whether the given timer has been initialized.
 *
 * @param timer     Address of timer.
 *
 * @retval CHIP_OS_OK Timer is initialized.
 * @retval CHIP_OS_EINVAL Timer is invalid.
 */
chip_os_error_t chip_os_timer_inited(struct chip_os_timer * timer);

bool chip_os_timer_is_active(struct chip_os_timer * timer);

chip_os_time_t chip_os_timer_get_ticks(struct chip_os_timer * timer);

chip_os_time_t chip_os_timer_remaining_ticks(struct chip_os_timer * timer, chip_os_time_t time);

void chip_os_timer_arg_set(struct chip_os_timer * timer, void * arg);

void * chip_os_timer_arg_get(struct chip_os_timer * timer);

/*
 * Time functions
 */

/**
 * @brief Get system uptime.
 *
 * This routine returns the elapsed time since the system booted,
 * in milliseconds.
 *
 * @return Current uptime.
 */
chip_os_time_t chip_os_time_get(void);

chip_os_time_t chip_os_time_get_ms(void);

chip_os_time_t chip_os_time_ms_to_ticks(chip_os_time_t ms);

chip_os_time_t chip_os_time_ticks_to_ms(chip_os_time_t ticks);

/*
 * Task functions
 */

/**
 * @brief Create a task.
 *
 * This routine initializes a task, then schedules it for immediate execution.
 * Typically, tasks are created and then started with @a chip_os_sched_start.
 *
 * @param t Pointer to uninitialized struct chip_os_task.
 * @param name Debugging name of the task.
 * @param func Task entry function.
 * @param arg Argument parameter pointer.
 * @param prio task priority.
 * @param stack_size Stack size in bytes.
 *
 * @retval CHIP_OS_OK Task is initialized.
 * @retval CHIP_OS_ERROR Task is invalid.
 */
chip_os_error_t chip_os_task_init(struct chip_os_task * t, const char * name, chip_os_task_func_t func, void * arg, uint8_t prio,
                                  uint16_t stack_size);

/**
 * @brief Yield the current task.
 *
 * This routine causes the current task to yield execution to another
 * task of the same or higher priority. If there are no other ready tasks
 * of the same or higher priority, the routine returns immediately.
 *
 * @return N/A
 */
void chip_os_task_yield(void);

/**
 * @brief Put the current thread to sleep.
 *
 * This routine puts the current thread to sleep for @a duration
 * [ticks].
 *
 * @param ticks Number of CPU ticks to sleep.
 *
 * @return N/A
 */
void chip_os_task_sleep(chip_os_time_t ticks);

void chip_os_task_sleep_ms(chip_os_time_t ms);

/**
 * @brief Get task ID of the current task.
 *
 * @return ID of current task.
 */
void * chip_os_get_current_task_id(void);

/**
 * @brief Abort a task.
 *
 * This routine permanently stops execution of @a task. The task is taken
 * off all kernel queues it is part of (i.e. the ready queue, the timeout
 * queue, or a kernel object wait queue). However, any kernel resources the
 * task might currently own (such as mutexes or memory blocks) are not
 * released. It is the responsibility of the caller of this routine to ensure
 * all necessary cleanup is performed.
 *
 * @param task ID of task to abort.
 *
 * @retval CHIP_OS_OK Task was removed.
 * @retval CHIP_OS_ERROR Problem removing task.
 */
chip_os_error_t chip_os_task_remove(struct chip_os_task * t);

/*
 * Scheduler functions
 */

/**
 * @brief Start the task scheduler.
 *
 * This routine will begin execution of all initialized @a tasks.
 *
 * @note This routine will never return.
 */
void chip_os_sched_start(void);

/**
 * @brief Returns whether the task scheduler has been started.
 *
 * @return true is started, false otherwise.
 */
bool chip_os_sched_started(void);

#ifdef __cplusplus
}
#endif

#endif /* CHIP_OSAL_H */
