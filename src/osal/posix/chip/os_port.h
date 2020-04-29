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

#ifndef CHIP_OS_PORT_H
#define CHIP_OS_PORT_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHIP_OS_OS_ALIGNMENT 4

#define CHIP_OS_TIME_FOREVER INT32_MAX
#define CHIP_OS_TIME_NO_WAIT 0

#define SYSINIT_PANIC_MSG(msg) __assert_fail(msg, __FILE__, __LINE__, __func__)

#define SYSINIT_PANIC_ASSERT_MSG(rc, msg)                                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(rc))                                                                                                                 \
        {                                                                                                                          \
            SYSINIT_PANIC_MSG(msg);                                                                                                \
        }                                                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* CHIP_OS_PORT_H */
