/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          Platform-specific configuration overrides for CHIP on
 *          RT-Thread platform.
 */

#pragma once

// ==================== Security Configuration Overrides ====================

#define PENDING_EVENT_FLAG			(1 << 0)
#define PENDING_EVENT_ISR_FLAG		(1 << 1)

#ifndef CHIP_CONFIG_RTTHREAD_USE_STATIC_TASK
#define CHIP_CONFIG_RTTHREAD_USE_STATIC_TASK 1
#endif

#ifndef CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX
#define CHIP_CONFIG_RTTHREAD_USE_STATIC_MUTEX 1
#endif

#ifndef CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT
#define CHIP_CONFIG_RTTHREAD_USE_STATIC_EVENT 1
#endif