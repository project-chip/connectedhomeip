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
  Unit tests for the chip_os_eventq api:

  void chip_os_eventq_init(struct chip_os_eventq *, size_t size);
  void chip_os_eventq_put(struct chip_os_eventq *, struct chip_os_event *);
  struct chip_os_event *chip_os_eventq_get_no_wait(struct chip_os_eventq *evq);
  struct chip_os_event *chip_os_eventq_get(struct chip_os_eventq *);
  void chip_os_eventq_run(struct chip_os_eventq *evq);
  struct chip_os_event *chip_os_eventq_poll(struct chip_os_eventq **, int, chip_os_time_t);
  struct chip_os_eventq *chip_os_eventq_dflt_get(void);
*/

#include <chip/osal.h>

#include "test_util.h"

#define TEST_ARGS_VALUE (55)
#define TEST_STACK_SIZE (1024)
#define TEST_EVENTQ_SIZE (32)

static bool s_tests_running = true;
static struct chip_os_task s_task_runner;

static struct chip_os_eventq s_eventq;
static struct chip_os_event s_event;
static int s_event_args = TEST_ARGS_VALUE;

void on_event(struct chip_os_event * ev)
{
    VerifyOrQuit(ev->ev_arg == &s_event_args, "callout: wrong args passed");

    VerifyOrQuit(*(int *) ev->ev_arg == TEST_ARGS_VALUE, "callout: args corrupted");

    s_tests_running = false;
}

int test_init(void)
{
    // VerifyOrQuit(!chip_os_eventq_inited(&s_eventq), "eventq: empty q initialized");
    chip_os_eventq_init(&s_eventq, TEST_EVENTQ_SIZE);
    // VerifyOrQuit(chip_os_eventq_inited(&s_eventq), "eventq: not initialized");

    return PASS;
}

int test_run(void)
{
    while (s_tests_running)
    {
        chip_os_eventq_run(&s_eventq);
    }

    return PASS;
}

int test_put(void)
{
    s_event.ev_cb  = on_event;
    s_event.ev_arg = &s_event_args;
    chip_os_eventq_put(&s_eventq, &s_event);
    return PASS;
}

int test_get_no_wait(void)
{
    // struct chip_os_event *ev = chip_os_eventq_get_no_wait(&s_eventq);
    return FAIL;
}

int test_get(void)
{
    struct chip_os_event * ev = chip_os_eventq_get(&s_eventq, CHIP_OS_TIME_FOREVER);

    VerifyOrQuit(ev == &s_event, "callout: wrong event passed");

    return PASS;
}

void * task_test_runner(void * args)
{
    SuccessOrQuit(test_init(), "eventq_init failed");
    SuccessOrQuit(test_put(), "eventq_put failed");
    SuccessOrQuit(test_get(), "eventq_get failed");
    SuccessOrQuit(test_put(), "eventq_put failed");
    SuccessOrQuit(test_run(), "eventq_run failed");

    printf("All tests passed\n");
    exit(PASS);

    return NULL;
}

int main(void)
{
    SuccessOrQuit(chip_os_task_init(&s_task_runner, "task_test_runner", task_test_runner, NULL, 1, 0), "task: error initializing");

    chip_os_sched_start();

    /* main never returns */

    exit(FAIL);
}
