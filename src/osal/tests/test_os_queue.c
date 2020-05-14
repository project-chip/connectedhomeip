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
  Unit tests for the chip_os_queue api:

  chip_os_error_t  chip_os_queue_init(struct chip_os_queue *evq,
                                      size_t msg_size,
                                      size_t max_msgs);
  chip_os_error_t chip_os_queue_get(struct chip_os_queue *evq,
                                    void *data,
                                    chip_os_time_t tmo);
  chip_os_error_t chip_os_queue_put(struct chip_os_queue *evq, void *data);
  bool chip_os_queue_is_empty(struct chip_os_queue *evq);
  int chip_os_queue_inited(const struct chip_os_queue *evq);
*/

#include <chip/osal.h>

#include "test_util.h"

#define TEST_ITERATIONS (100)
#define TEST_TASK_PRIO (1)
#define TEST_STACK_SIZE (1024)
#define TEST_QUEUE_SIZE (32)

typedef struct
{
    int _type;
    int _value;
    int _param;
} my_msg_t;

static struct chip_os_task s_task1;
static struct chip_os_task s_task2;

struct chip_os_queue s_task1_queue;
struct chip_os_queue s_task2_queue;

static struct chip_os_queue s_queue;

static my_msg_t s_task1_msg_rx;
static my_msg_t s_task2_msg_rx;

my_msg_t s_task1_msgs_src[] = {
    { ._type = 0, ._value = 0, ._param = 10000 },
    { ._type = 1, ._value = 1, ._param = 10001 },
    { ._type = 2, ._value = 2, ._param = 10002 },
    { ._type = 3, ._value = 3, ._param = 10003 },
};

my_msg_t s_task2_msgs_src[] = {
    { ._type = 0, ._value = 20, ._param = 20000 },
    { ._type = 1, ._value = 21, ._param = 20001 },
    { ._type = 2, ._value = 22, ._param = 20002 },
    { ._type = 3, ._value = 23, ._param = 20003 },
};

#define TEST_MSGS_COUNT (sizeof(s_task1_msgs_src) / sizeof(my_msg_t))

void test_queue_init(struct chip_os_queue * q)
{
    VerifyOrQuit(!chip_os_queue_inited(q), "queue: q already initialized");
    chip_os_queue_init(q, sizeof(my_msg_t), TEST_QUEUE_SIZE);
    VerifyOrQuit(chip_os_queue_inited(q), "queue: not initialized");
}

void test_put(void)
{
    chip_os_queue_put(&s_queue, &s_task1_msgs_src[0]);
}

void test_get_verify(my_msg_t * msg_rx, my_msg_t * msg_src)
{
    VerifyOrQuit(msg_rx->_type == msg_src->_type, "queue: wrong msg type passed");
    VerifyOrQuit(msg_rx->_value == msg_src->_value, "queue: wrong msg value passed");
    VerifyOrQuit(msg_rx->_param == msg_src->_param, "queue: wrong msg param passed");
}

void test_get(void)
{
    chip_os_queue_get(&s_queue, &s_task1_msg_rx, CHIP_OS_TIME_FOREVER);
    test_get_verify(&s_task1_msg_rx, &s_task1_msgs_src[0]);
}

void test_queue_units()
{
    test_queue_init(&s_queue);
    test_put();
    test_get();
    test_put();
}

/* Task 1 handler function */
void * task1_run(void * arg)
{
    int i, idx;

    test_queue_units();

    for (i = 0; i < TEST_ITERATIONS; i++)
    {
        idx = i % TEST_MSGS_COUNT;

        /* Post to task2 msg queue */
        SuccessOrQuit(chip_os_queue_put(&s_task2_queue, &s_task2_msgs_src[idx]),
                      "chip_os_queue_put: error posting to s_task1_queue.");

        /* Wait on task1 msg queue. */
        SuccessOrQuit(chip_os_queue_get(&s_task1_queue, &s_task1_msg_rx, CHIP_OS_TIME_FOREVER),
                      "chip_os_queue_get: error waiting for s_task2_queue.");

        test_get_verify(&s_task1_msg_rx, &s_task1_msgs_src[idx]);
        TEST_LOG("task1: recieved msg.value = %d\n", s_task1_msg_rx._value);
    }

    printf("All tests passed\n");
    exit(PASS);
}

/* Task 2 handler function */
void * task2_run(void * arg)
{
    int i = 0;
    int idx;

    while (1)
    {
        idx = i++ % TEST_MSGS_COUNT;

        /* Wait on task2 msg queue. */
        SuccessOrQuit(chip_os_queue_get(&s_task2_queue, &s_task2_msg_rx, CHIP_OS_TIME_FOREVER),
                      "chip_os_queue_get: error waiting for s_task1_queue.");

        test_get_verify(&s_task2_msg_rx, &s_task2_msgs_src[idx]);
        TEST_LOG("task2: recieved msg.value = %d\n", s_task2_msg_rx._value);

        /* Post to task1 msg queue */
        SuccessOrQuit(chip_os_queue_put(&s_task1_queue, &s_task1_msgs_src[idx]),
                      "chip_os_queue_put: error posting to s_task1_queue.");
    }

    /* Never returns -- task1 will exit the test. */
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
     * Call task specific initialization functions to initialize any shared objects
     * before initializing the tasks with the OS.
     */
    test_queue_init(&s_task1_queue);

    test_queue_init(&s_task2_queue);

    /*
     * Initialize tasks 1 and 2 with the OS.
     */
    chip_os_task_init(&s_task1, "task1", task1_run, NULL, TEST_TASK_PRIO, TEST_STACK_SIZE);

    chip_os_task_init(&s_task2, "task2", task2_run, NULL, TEST_TASK_PRIO, TEST_STACK_SIZE);

    chip_os_sched_start();

    /* main never returns */

    exit(FAIL);
}
