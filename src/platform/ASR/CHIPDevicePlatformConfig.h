/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

// ==================== Platform Adaptations ====================

#define CHIP_DEVICE_CONFIG_LWIP_WIFI_STATION_IF_NAME "asr"

#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "ASR"
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8001
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "asr582x"
#define CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE 1024 * 8
#define CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY 1
#define CHIP_DEVICE_CONFIG_ENABLE_TEST_SETUP_PARAMS 1
#define CHIP_DEVICE_CONFIG_TEST_MANUFACTURY_DATE "2023-07-07"

#if defined(CFG_PLF_RV32) || defined(CFG_PLF_DUET)
// ASR582X/ASR595X support Wi-Fi and BLE
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1
#else
// ASR550X support Wi-Fi only
#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0
#endif

#define CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE 25

#define CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART 1

#define CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART 1

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

#define CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX 70

#define CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX 1800
