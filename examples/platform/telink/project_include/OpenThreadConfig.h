/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#pragma once

// Include Zephyr-supplied OpenThread configuration

#include "openthread-core-zephyr-config.h"

// Project-specific settings
#include "openthread/platform/logging.h"

/* Number of message buffers reduced to save RAM */
#undef OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS
#if CONFIG_PM
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 22
#else
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 44
#endif

#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_DEBUG_UART

#undef OPENTHREAD_CONFIG_LOG_LEVEL
#if (CONFIG_LOG_MODE_MINIMAL)
#define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_NONE
#else
#define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_DEBG
#endif

#define OPENTHREAD_CONFIG_ENABLE_DEBUG_UART 1

#undef OPENTHREAD_CONFIG_LOG_CLI
#define OPENTHREAD_CONFIG_LOG_CLI 0

#undef OPENTHREAD_CONFIG_LOG_PKT_DUMP
#define OPENTHREAD_CONFIG_LOG_PKT_DUMP 0

#undef OPENTHREAD_CONFIG_LOG_PLATFORM
#define OPENTHREAD_CONFIG_LOG_PLATFORM 0

#undef OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL
#define OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL 0
