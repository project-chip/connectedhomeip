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

/**
  Unit tests for the chip_os_timer api:

  void chip_os_timer_init(struct chip_os_timer * timer, chip_os_timer_fn cb, void * arg);
  void chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t duration);
  void chip_os_timer_start_ticks(struct chip_os_timer * timer, chip_os_time_t ticks);
  void chip_os_timer_stop(struct chip_os_timer * timer);
  bool chip_os_timer_is_active(struct chip_os_timer * timer);
  chip_os_time_t chip_os_timer_get_ticks(struct chip_os_timer * timer);
  chip_os_time_t chip_os_timer_remaining_ticks(struct chip_os_timer * timer, chip_os_time_t time);
  void chip_os_timer_arg_set(struct chip_os_timer * timer, void * arg);
  void * chip_os_timer_arg_get(struct chip_os_timer * timer);
*/

#include <limits.h>

#include <chip/osal.h>

#include "test_util.h"

#define TEST_ITERATIONS (100)
#define TEST_TASK_PRIO (1)
#define TEST_STACK_SIZE (1024)

#define TEST_TIMER_MARGIN (10)
#define TEST_TIMER1_DURATION (1000)

static struct chip_os_task s_task1;

struct chip_os_timer s_timer1;

static chip_os_time_t s_timer1_start;

static void test_timer1_fired(void * arg)
{
    struct chip_os_timer * t = (struct chip_os_timer *) arg;
    chip_os_time_t now       = chip_os_time_get();
    chip_os_stime_t delta    = now - s_timer1_start;

    TEST_LOG("test_timer fired delta=%d\n", delta);
    VerifyOrQuit((delta >= TEST_TIMER1_DURATION), "timer: duration too short");
    VerifyOrQuit((delta < (TEST_TIMER1_DURATION + TEST_TIMER_MARGIN)), "timer: duration too long");
    VerifyOrQuit(!chip_os_timer_is_active(t), "timer: fired, but still active");
    VerifyOrQuit(chip_os_timer_arg_get(t) == t, "timer: arg incorrect");

    printf("All tests passed\n");
    exit(PASS);
}

void test_timer1(struct chip_os_timer * t)
{
    s_timer1_start = chip_os_time_get();
    chip_os_timer_init(t, test_timer1_fired, t);
    chip_os_timer_start(t, TEST_TIMER1_DURATION);
}

/* Task 1 handler function */
void * task1_run(void * arg)
{
    test_timer1(&s_timer1);

    /* Wait for timers to trigger exit. */
    while (1)
    {
        chip_os_task_yield();
    }
}

/**
 * init_app_tasks
 *
 * This function performs initializations that are required before tasks run.
 *
 * @return int 0 success; error otherwise.
 */
int main(void)
{
    /*
     * Initialize tasks 1 and 2 with the OS.
     */
    chip_os_task_init(&s_task1, "task1", task1_run, NULL, TEST_TASK_PRIO, TEST_STACK_SIZE);

    chip_os_sched_start();

    /* main never returns */

    exit(FAIL);
}
