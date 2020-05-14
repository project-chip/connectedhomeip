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

#include <chip/osal.h>

#include <assert.h>

void chip_os_task_dispatch(void * arg)
{
    struct chip_os_task * task = (struct chip_os_task *) arg;

    assert(task);
    assert(task->func);
    task->func(task->arg);
}

chip_os_error_t chip_os_task_init(struct chip_os_task * task, const char * name, chip_os_task_func_t func, void * arg, uint8_t prio,
                                  uint16_t stack_size)
{
    chip_os_base_t err;

    if ((task == NULL) || (func == NULL))
    {
        return CHIP_OS_INVALID_PARAM;
    }

    task->func = func;
    task->arg  = arg;

    err = xTaskCreate(chip_os_task_dispatch, name, stack_size / sizeof(chip_os_base_t), task, prio, &task->handle);

    return (err == pdPASS) ? CHIP_OS_OK : CHIP_OS_ENOMEM;
}
