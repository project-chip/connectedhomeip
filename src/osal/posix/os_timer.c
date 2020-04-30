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

#include <signal.h>

#include <chip/osal.h>
#include "os_utils.h"

#ifdef __APPLE__

#define DEFAULT_TIMER_LEEWAY 100000 //< 100 [us]

static void chip_os_timer_cb(void * arg)
{
    struct chip_os_timer * timer = (struct chip_os_timer *) arg;
    assert(timer);

    chip_os_timer_stop(timer);

    timer->tm_cb(timer->tm_arg);
}

chip_os_error_t chip_os_timer_init(struct chip_os_timer * timer, chip_os_timer_fn * tm_cb, void * tm_arg)
{
    dispatch_source_t dispatch = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler_f(dispatch, chip_os_timer_cb);
    dispatch_set_context(dispatch, timer);

    /* Initialize the timer. */
    memset(timer, 0, sizeof(*timer));
    timer->tm_cb     = tm_cb;
    timer->tm_arg    = tm_arg;
    timer->tm_active = false;
    timer->tm_timer  = dispatch;

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t ticks)
{
    chip_os_error_t err;
    dispatch_source_t dispatch;
    chip_os_time_t delay_ns;

    err = chip_os_timer_inited(timer);
    SuccessOrExit(err);

    dispatch = timer->tm_timer;
    err      = (dispatch == NULL) ? CHIP_OS_EINVAL : CHIP_OS_OK;
    SuccessOrExit(err);

    timer->tm_ticks = chip_os_time_get() + ticks;
    delay_ns        = ticks * (1000000000.0 / CHIP_OS_TICKS_PER_SEC);
    dispatch_source_set_timer(dispatch, dispatch_time(DISPATCH_TIME_NOW, delay_ns), DISPATCH_TIME_FOREVER, DEFAULT_TIMER_LEEWAY);
    dispatch_resume(dispatch);
    timer->tm_active = true;

exit:
    return err;
}

chip_os_error_t chip_os_timer_stop(struct chip_os_timer * timer)
{
    chip_os_error_t err;

    err = chip_os_timer_inited(timer);
    SuccessOrExit(err);

    err = (timer->tm_timer == NULL) ? CHIP_OS_EINVAL : CHIP_OS_OK;
    SuccessOrExit(err);

    dispatch_source_cancel(timer->tm_timer);
    timer->tm_active = false;

exit:
    return err;
}

chip_os_time_t chip_os_timer_remaining_ticks(struct chip_os_timer * timer, chip_os_time_t now)
{
    // TODO: properly determine ticks remaining if possible or remove API if unused.
    return 0;
}

#else

static void chip_os_timer_cb(union sigval sv)
{
    struct chip_os_timer * timer = (struct chip_os_timer *) sv.sival_ptr;
    assert(timer);

    timer->tm_active = false;
    timer->tm_cb(timer->tm_arg);
}

chip_os_error_t chip_os_timer_init(struct chip_os_timer * timer, chip_os_timer_fn * tm_cb, void * tm_arg)
{
    int ret;
    struct sigevent event;

    /* Initialize the timer. */
    memset(timer, 0, sizeof(*timer));
    timer->tm_cb     = tm_cb;
    timer->tm_arg    = tm_arg;
    timer->tm_active = false;

    event.sigev_notify            = SIGEV_THREAD;
    event.sigev_value.sival_ptr   = timer; // put timer obj in signal args
    event.sigev_notify_function   = chip_os_timer_cb;
    event.sigev_notify_attributes = NULL;

    ret = timer_create(CLOCK_REALTIME, &event, &timer->tm_timer);
    return (ret) ? CHIP_OS_ERROR : CHIP_OS_OK;
}

chip_os_error_t chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t ticks)
{
    struct itimerspec its;
    chip_os_error_t err;
    int ret;

    err = chip_os_timer_inited(timer);
    SuccessOrExit(err);

    if (ticks == 0)
    {
        ticks = 1;
    }

    timer->tm_ticks = chip_os_time_get() + ticks;

    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = 0; // one shot
    its.it_value.tv_sec     = (ticks / 1000);
    its.it_value.tv_nsec    = (ticks % 1000) * 1000000; // expiration
    its.it_value.tv_nsec %= 1000000000;
    timer->tm_active = true;

    ret = timer_settime(timer->tm_timer, 0, &its, NULL);
    err = (ret) ? CHIP_OS_EINVAL : CHIP_OS_OK;
    SuccessOrExit(err);

exit:
    return err;
}

chip_os_error_t chip_os_timer_stop(struct chip_os_timer * timer)
{
    chip_os_error_t err;
    int ret;

    err = chip_os_timer_inited(timer);
    SuccessOrExit(err);

    struct itimerspec its;
    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec     = 0;
    its.it_value.tv_nsec    = 0;

    ret = timer_settime(timer->tm_timer, 0, &its, NULL);
    err = (ret) ? CHIP_OS_EINVAL : CHIP_OS_OK;
    SuccessOrExit(err);

    timer->tm_active = false;

exit:
    return err;
}

chip_os_time_t chip_os_timer_remaining_ticks(struct chip_os_timer * timer, chip_os_time_t now)
{
    chip_os_time_t rt;
    uint32_t exp;

    struct itimerspec its;
    timer_gettime(timer->tm_timer, &its);

    exp = its.it_value.tv_sec * 1000;

    if (exp > now)
    {
        rt = exp - now;
    }
    else
    {
        rt = 0;
    }

    return rt;
}

#endif // __APPLE__

chip_os_error_t chip_os_timer_start_ms(struct chip_os_timer * timer, chip_os_time_t duration)
{
    chip_os_time_t delta = chip_os_time_ms_to_ticks(duration);
    return chip_os_timer_start(timer, delta);
}

bool chip_os_timer_is_active(struct chip_os_timer * timer)
{
    // TODO: seek native posix method to determine whether timer_t is active.
    // TODO: fix bug where one-shot timer is still active after fired.
    return timer->tm_active;
}

chip_os_error_t chip_os_timer_inited(struct chip_os_timer * timer)
{
    return (timer->tm_timer == NULL) ? CHIP_OS_EINVAL : CHIP_OS_OK;
}

chip_os_time_t chip_os_timer_get_ticks(struct chip_os_timer * timer)
{
    return timer->tm_ticks;
}

void * chip_os_timer_arg_get(struct chip_os_timer * timer)
{
    return timer->tm_arg;
}

void chip_os_timer_arg_set(struct chip_os_timer * timer, void * arg)
{
    timer->tm_arg = arg;
}
