/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *    @file
 *          Defines timing helper functions.
 */

#pragma once

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t GetTick(void);

/* Time to kernel ticks */
uint32_t sec2tick(uint32_t sec);

uint32_t ms2tick(uint32_t ms);

uint32_t us2tick(uint32_t usec);

/* Kernel ticks to time */
uint64_t tick2sec(uint64_t tick);

uint64_t tick2ms(uint64_t tick);

uint64_t tick2us(uint64_t tick);

#ifdef __cplusplus
}
#endif
