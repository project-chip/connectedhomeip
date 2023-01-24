/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          on mbed platform.
 */

#pragma once

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0

#ifndef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING MBED_CONF_APP_VERSION_NUMBER_STR
#endif // CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION MBED_CONF_APP_VERSION_NUMBER
#endif // CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION

#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID MBED_CONF_APP_VENDOR_ID
#endif // CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID

#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID MBED_CONF_APP_PRODUCT_ID
#endif // CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 8192
#endif // CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE
