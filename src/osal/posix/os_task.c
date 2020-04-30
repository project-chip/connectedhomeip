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

#include <limits.h>

#include <chip/osal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a task.
 *
 * This function initializes the task structure pointed to by t,
 * clearing and setting it's stack pointer, provides sane defaults
 * and sets the task as ready to run, and inserts it into the operating
 * system scheduler.
 *
 * @param t The task to initialize
 * @param name The name of the task to initialize
 * @param func The task function to call
 * @param arg The argument to pass to this task function
 * @param prio The priority at which to run this task
 * @param stack_size The overall size of the task's stack.
 *
 * @return 0 on success, non-zero on failure.
 */
int chip_os_task_init(struct chip_os_task * t, const char * name, chip_os_task_func_t func, void * arg, uint8_t prio,
                      uint16_t stack_size)
{
    int err;
    if ((t == NULL) || (func == NULL))
    {
        return CHIP_OS_INVALID_PARAM;
    }

    err = pthread_attr_init(&t->attr);
    if (err)
        return err;
    err = pthread_attr_getschedparam(&t->attr, &t->param);
    if (err)
        return err;
    err = pthread_attr_setschedpolicy(&t->attr, SCHED_RR);
    if (err)
        return err;
    stack_size = (stack_size < PTHREAD_STACK_MIN) ? PTHREAD_STACK_MIN : stack_size;
    err        = pthread_attr_setstacksize(&t->attr, stack_size);
    if (err)
        return err;
    t->param.sched_priority = prio;
    err                     = pthread_attr_setschedparam(&t->attr, &t->param);
    if (err)
        return err;

    t->name = name;
    err     = pthread_create(&t->handle, &t->attr, func, arg);

    return err;
}

/*
 * Removes specified task
 * XXX
 * NOTE: This interface is currently experimental and not ready for common use
 */
int chip_os_task_remove(struct chip_os_task * t)
{
    return pthread_cancel(t->handle);
}

/**
 * Return the number of tasks initialized.
 *
 * @return number of tasks initialized
 */
uint8_t chip_os_task_count(void)
{
    return 0;
}

void * chip_os_get_current_task_id(void)
{
    return (void *) pthread_self();
}

void chip_os_task_yield(void)
{
    sched_yield();
}

void chip_os_task_sleep(chip_os_time_t ticks)
{
    struct timespec sleep_time;
    long ms    = chip_os_time_ticks_to_ms(ticks);
    uint32_t s = ms / 1000;

    ms -= s * 1000;
    sleep_time.tv_sec  = s;
    sleep_time.tv_nsec = ms * 1000000;

    nanosleep(&sleep_time, NULL);
}

bool chip_os_sched_started(void)
{
    return true;
}

void chip_os_sched_start(void)
{
#ifdef __APPLE__
    /* Start the main queue */
    dispatch_main();
#else
    while (true)
    {
        chip_os_task_yield();
    }
#endif // __APPLE__

    assert(true);
}

#ifdef __cplusplus
}
#endif
