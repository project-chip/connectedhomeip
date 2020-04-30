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
  Unit tests for the Mutex api (chip_os_mutex):

  chip_os_error_t chip_os_mutex_init(struct chip_os_mutex *sem);
  chip_os_error_t chip_os_mutex_give(struct chip_os_mutex *sem);
  chip_os_error_t chip_os_mutex_take(struct chip_os_mutex *sem);
*/

#include <chip/osal.h>

#include "test_util.h"

#define TEST_ITERATIONS 10
#define TASK1_VALUE 0x11
#define TASK2_VALUE 0x22

#define TASK1_PRIO 1
#define TASK2_PRIO 1

#define TASK1_STACK_SIZE 1028
#define TASK2_STACK_SIZE 1028

static struct chip_os_task task1;
static struct chip_os_task task2;

struct chip_os_mutex task1_mtx;

uint8_t s_buffer[TEST_ITERATIONS];

void test_mutex_nested(uint8_t test_value, int task, bool recursive, int round)
{
    TEST_LOG("task %d: LOCK START ROUND %d\n", task, round);
    SuccessOrQuit(chip_os_mutex_take(&task1_mtx, CHIP_OS_TIME_FOREVER), "chip_os_mutex_take: error waiting for task1_mutex.");
    for (int i = 0; i < TEST_ITERATIONS; i++)
    {
        if (recursive)
        {
            TEST_LOG("task %d: take #%d RECURSIVE\n", task, i);
            SuccessOrQuit(chip_os_mutex_take(&task1_mtx, CHIP_OS_TIME_FOREVER),
                          "chip_os_mutex_take: error waiting for task1_mutex.");
        }
        // TEST_LOG("task %d: b[%d]=0x%02x --> 0x%02x\n", task, i, s_buffer[i], test_value);
        VerifyOrQuit(s_buffer[i] != test_value, "unexpected value: mutex did not protect resource");
        s_buffer[i] = test_value;
    }

    for (int i = 0; i < TEST_ITERATIONS; i++)
    {
        // TEST_LOG("task %d: b[%d]=0x%02x\n", task, i, s_buffer[i]);
        VerifyOrQuit(s_buffer[i] == test_value, "unexpected value: mutex did not protect resource");
        if (recursive)
        {
            TEST_LOG("task %d: give #%d RECURSIVE\n", task, i);
            SuccessOrQuit(chip_os_mutex_give(&task1_mtx), "chip_os_mutex_give: error releasing task1_mutex.");
        }
    }
    // Clear out buffer in case this task reclaims the resource again
    memset(&s_buffer, 0, sizeof(s_buffer));
    TEST_LOG("task %d: LOCK END ROUND %d\n", task, round);
    SuccessOrQuit(chip_os_mutex_give(&task1_mtx), "chip_os_mutex_give: error releasing task1_mutex.");

    chip_os_task_sleep(10);
}

/* Task 1 handler function */
void * task1_run(void * arg)
{
    for (int round = 0; round < TEST_ITERATIONS; round++)
    {
        bool recursive = (round % 5 == 0);
        test_mutex_nested(TASK1_VALUE, 1, recursive, round);
    }

    printf("All tests passed\n");
    exit(PASS);

    return NULL;
}

/* Task 2 handler function */
void * task2_run(void * arg)
{
    int i = 0;
    while (1)
    {
        bool recursive = (i % 5 == 0);
        test_mutex_nested(TASK2_VALUE, 2, recursive, i);
        i++;
    }

    return NULL;
}

/**
 * init_app_tasks
 *
 * This function performs initializations that are required before tasks run.
 *
 * @return int 0 success; error otherwise.
 */
static int init_app_tasks(void)
{
    memset(&s_buffer, 0, sizeof(s_buffer));
    SuccessOrQuit(chip_os_mutex_init(&task1_mtx), "chip_os_mutex_init: task1 returned error.");

    /*
     * Initialize tasks 1 and 2 with the OS.
     */
    chip_os_task_init(&task1, "task1", task1_run, NULL, TASK1_PRIO, TASK1_STACK_SIZE);

    chip_os_task_init(&task2, "task2", task2_run, NULL, TASK2_PRIO, TASK2_STACK_SIZE);

    return 0;
}

/**
 * main
 *
 * The main function for the application. This function initializes the system and packages,
 * calls the application specific task initialization function, then waits and dispatches
 * events from the OS default event queue in an infinite loop.
 */
int main(int argc, char ** arg)
{
    /* Initialize application specific tasks */
    init_app_tasks();

    chip_os_sched_start();

    /* main never returns */

    exit(FAIL);
}
