/*
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

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 1
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#else
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION 0
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP 0
#endif

#ifndef CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#define CHIP_DEVICE_CONFIG_ENABLE_ETHERNET 0
#endif

#if CHIP_ENABLE_OPENTHREAD
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 1
#else
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT 0
#endif

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY 0
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_TELEMETRY_FULL 0

#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE (8 * 1024)
#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 50

#ifndef CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE
#define CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE 0
#endif

#if CHIP_DEVICE_LAYER_TARGET_BL602
#define CHIP_CONFIG_FREERTOS_USE_STATIC_TASK 1
#define CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE 1
#define CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE 1
#elif CHIP_DEVICE_LAYER_TARGET_BL702
#define CHIP_CONFIG_FREERTOS_USE_STATIC_TASK 1
#elif CHIP_DEVICE_LAYER_TARGET_BL702L
#define CHIP_CONFIG_FREERTOS_USE_STATIC_TASK 1
#endif
