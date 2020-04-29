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

/**
  Unit tests for the chip_os_callout api:

  void chip_os_callout_init(struct chip_os_callout *cf, struct chip_os_eventq *evq,
                       chip_os_event_fn *ev_cb, void *ev_arg);
  int chip_os_callout_reset(struct chip_os_callout *, int32_t);
  int chip_os_callout_queued(struct chip_os_callout *c);
  void chip_os_callout_stop(struct chip_os_callout *c);
*/

#include <chip/osal.h>

#include "test_util.h"

#define TEST_ARGS_VALUE (55)
#define TEST_INTERVAL (100)
#define TEST_EVENTQ_SIZE (32)

static bool s_tests_running = true;
static struct chip_os_task s_task;
static struct chip_os_callout s_callout;
static int s_callout_args = TEST_ARGS_VALUE;

static struct chip_os_eventq s_eventq;

void on_callout(struct chip_os_event * ev)
{
    VerifyOrQuit(ev->ev_arg == &s_callout_args, "callout: wrong args passed");

    VerifyOrQuit(*(int *) ev->ev_arg == TEST_ARGS_VALUE, "callout: args corrupted");

    s_tests_running = false;
}

int test_init(void)
{
    chip_os_callout_init(&s_callout, &s_eventq, on_callout, &s_callout_args);
    return PASS;
}

int test_queued(void)
{
    // VerifyOrQuit(chip_os_callout_queued(&s_callout),
    //	 "callout: not queued when expected");
    return PASS;
}

int test_reset(void)
{
    return chip_os_callout_reset(&s_callout, TEST_INTERVAL);
}

int test_stop(void)
{
    return PASS;
}

void * test_task_run(void * args)
{
    SuccessOrQuit(test_init(), "callout_init failed");
    SuccessOrQuit(test_queued(), "callout_queued failed");
    SuccessOrQuit(test_reset(), "callout_reset failed");

    while (s_tests_running)
    {
        chip_os_eventq_run(&s_eventq);
    }

    printf("All tests passed\n");
    exit(PASS);

    return NULL;
}

int main(void)
{
    chip_os_eventq_init(&s_eventq, TEST_EVENTQ_SIZE);

    SuccessOrQuit(chip_os_task_init(&s_task, "s_task", test_task_run, NULL, 1, 0), "task: error initializing");

    chip_os_sched_start();

    /* main never returns */

    exit(FAIL);
}
