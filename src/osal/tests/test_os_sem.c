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
  Unit tests for the Semaphore api (chip_os_sem):

  chip_os_error_t chip_os_sem_init(struct chip_os_sem *sem, uint16_t tokens);
  chip_os_error_t chip_os_sem_give(struct chip_os_sem *sem);
  chip_os_error_t chip_os_sem_take(struct chip_os_sem *sem, uint32_t timeout);
*/

#include <chip/osal.h>

#include "test_util.h"

#define TEST_ITERATIONS 10

#define TASK1_PRIO CHIP_OS_PRIORITY_APP
#define TASK2_PRIO CHIP_OS_PRIORITY_APP

#define TASK1_STACK_SIZE 1028
#define TASK2_STACK_SIZE 1028

static struct chip_os_task task1;
static struct chip_os_task task2;

struct chip_os_sem task1_sem;
struct chip_os_sem task2_sem;

/* Task 1 handler function */
void * task1_run(void * arg)
{
    for (int i = 0; i < TEST_ITERATIONS; i++)
    {
        /* Release semaphore to task 2 */
        SuccessOrQuit(chip_os_sem_give(&task1_sem), "chip_os_sem_give: error releasing task2_sem.");

        /* Wait for semaphore from task 2 */
        SuccessOrQuit(chip_os_sem_take(&task2_sem, CHIP_OS_TIME_FOREVER), "chip_os_sem_take: error waiting for task2_sem.");
    }

    printf("All tests passed\n");
    exit(PASS);

    return NULL;
}

/* Task 2 handler function */
void * task2_run(void * arg)
{
    while (1)
    {
        /* Wait for semaphore from task1 */
        SuccessOrQuit(chip_os_sem_take(&task1_sem, CHIP_OS_TIME_FOREVER), "chip_os_sem_take: error waiting for task1_sem.");

        /* Release task2 semaphore */
        SuccessOrQuit(chip_os_sem_give(&task2_sem), "chip_os_sem_give: error releasing task1_sem.");
    }

    return NULL;
}

/* Initialize task 1 exposed data objects */
void task1_init(void)
{
    /* Initialize task1 semaphore */
    SuccessOrQuit(chip_os_sem_init(&task1_sem, 0), "chip_os_sem_init: task1 returned error.");
}

/* Initialize task 2 exposed data objects */
void task2_init(void)
{
    /* Initialize task1 semaphore */
    SuccessOrQuit(chip_os_sem_init(&task2_sem, 0), "chip_os_sem_init: task2 returned error.");
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
    /*
     * Call task specific initialization functions to initialize any shared objects
     * before initializing the tasks with the OS.
     */
    task1_init();
    task2_init();

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
