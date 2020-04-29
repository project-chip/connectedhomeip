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

#include <chip/osal.h>
#include "RingPthread.h"

extern "C" {

typedef RingPthread ring_t;

void chip_os_eventq_init(struct chip_os_eventq * evq, size_t max_msgs)
{
    evq->q = new ring_t(sizeof(struct chip_os_event *), max_msgs);
}

bool chip_os_eventq_is_empty(struct chip_os_eventq * evq)
{
    ring_t * q = static_cast<ring_t *>(evq->q);

    if (q->size())
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int chip_os_eventq_inited(const struct chip_os_eventq * evq)
{
    return (evq->q != NULL);
}

void chip_os_eventq_put(struct chip_os_eventq * evq, struct chip_os_event * ev)
{
    ring_t * q = static_cast<ring_t *>(evq->q);

    if (ev->ev_queued)
    {
        return;
    }

    ev->ev_queued = 1;
    q->put((uint8_t *) &ev);
}

struct chip_os_event * chip_os_eventq_get(struct chip_os_eventq * evq, chip_os_time_t tmo)
{
    struct chip_os_event * ev;
    ring_t * q = static_cast<ring_t *>(evq->q);

    q->get((uint8_t *) &ev, tmo);

    if (ev)
    {
        ev->ev_queued = 0;
    }

    return ev;
}

void chip_os_eventq_run(struct chip_os_eventq * evq)
{
    struct chip_os_event * ev;

    ev = chip_os_eventq_get(evq, CHIP_OS_TIME_FOREVER);
    chip_os_event_run(ev);
}

// ========================================================================
//                         Event Implementation
// ========================================================================

void chip_os_event_init(struct chip_os_event * ev, chip_os_event_fn * fn, void * arg)
{
    memset(ev, 0, sizeof(*ev));
    ev->ev_cb  = fn;
    ev->ev_arg = arg;
}

bool chip_os_event_is_queued(struct chip_os_event * ev)
{
    return ev->ev_queued;
}

void * chip_os_event_get_arg(struct chip_os_event * ev)
{
    return ev->ev_arg;
}

void chip_os_event_set_arg(struct chip_os_event * ev, void * arg)
{
    ev->ev_arg = arg;
}

void chip_os_event_run(struct chip_os_event * ev)
{
    assert(ev->ev_cb != NULL);

    ev->ev_cb(ev);
}

} // extern "C"
