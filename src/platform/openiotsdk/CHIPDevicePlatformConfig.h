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
 *          Platform-specific configuration overrides for the chip Device Layer
 *          on Open IOT SDK platform.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC 0

#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0

// ========== Platform-specific Configuration Overrides =========

#define CHIP_DEVICE_CONFIG_LOG_PROVISIONING_HASH 0

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
