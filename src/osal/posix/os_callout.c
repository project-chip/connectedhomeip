/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Apache Software Foundation (ASF)
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

static void chip_os_callout_timer_cb(union sigval sv)
{
    struct chip_os_callout * c = (struct chip_os_callout *) sv.sival_ptr;
    assert(c);

    if (c->c_evq)
    {
        chip_os_eventq_put(c->c_evq, &c->c_ev);
    }
    else
    {
        c->c_ev.ev_cb(&c->c_ev);
    }
}

void chip_os_callout_init(struct chip_os_callout * c, struct chip_os_eventq * evq, chip_os_event_fn * ev_cb, void * ev_arg)
{
    struct sigevent event;

    /* Initialize the callout. */
    memset(c, 0, sizeof(*c));
    c->c_ev.ev_cb  = ev_cb;
    c->c_ev.ev_arg = ev_arg;
    c->c_evq       = evq;
    c->c_active    = false;

    event.sigev_notify            = SIGEV_THREAD;
    event.sigev_value.sival_ptr   = c; // put callout obj in signal args
    event.sigev_notify_function   = chip_os_callout_timer_cb;
    event.sigev_notify_attributes = NULL;

    timer_create(CLOCK_REALTIME, &event, &c->c_timer);
}

bool chip_os_callout_is_active(struct chip_os_callout * c)
{
    // TODO: seek native posix method to determine whether timer_t is active.
    // TODO: fix bug where one-shot timer is still active after fired.
    return c->c_active;
}

int chip_os_callout_inited(struct chip_os_callout * c)
{
    return (c->c_timer != NULL);
}

chip_os_error_t chip_os_callout_reset(struct chip_os_callout * c, chip_os_time_t ticks)
{
    struct itimerspec its;

    if (ticks == 0)
    {
        ticks = 1;
    }

    c->c_ticks = chip_os_time_get() + ticks;

    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = 0; // one shot
    its.it_value.tv_sec     = (ticks / 1000);
    its.it_value.tv_nsec    = (ticks % 1000) * 1000000; // expiration
    its.it_value.tv_nsec %= 1000000000;
    c->c_active = true;
    timer_settime(c->c_timer, 0, &its, NULL);

    return CHIP_OS_OK;
}

int chip_os_callout_queued(struct chip_os_callout * c)
{
    struct itimerspec its;
    timer_gettime(c->c_timer, &its);

    return ((its.it_value.tv_sec > 0) || (its.it_value.tv_nsec > 0));
}

void chip_os_callout_stop(struct chip_os_callout * c)
{
    if (!chip_os_callout_inited(c))
    {
        return;
    }

    struct itimerspec its;
    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec     = 0;
    its.it_value.tv_nsec    = 0;
    timer_settime(c->c_timer, 0, &its, NULL);
    c->c_active = false;
}

chip_os_time_t chip_os_callout_get_ticks(struct chip_os_callout * co)
{
    return co->c_ticks;
}

void chip_os_callout_set_arg(struct chip_os_callout * co, void * arg)
{
    co->c_ev.ev_arg = arg;
}

uint32_t chip_os_callout_remaining_ticks(struct chip_os_callout * co, chip_os_time_t now)
{
    chip_os_time_t rt;
    uint32_t exp;

    struct itimerspec its;
    timer_gettime(co->c_timer, &its);

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
