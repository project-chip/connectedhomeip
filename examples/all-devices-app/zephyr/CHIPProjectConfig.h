/*
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#endif

#define CHIP_DEVICE_CONFIG_CHIP_TASK_NAME "CHIP"
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY (K_PRIO_PREEMPT(10))
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 10240

#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#define CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES 1
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8000
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_UTC_TIMESTAMPS 1
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)
#define CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL (2000_ms32)
