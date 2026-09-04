/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#define CHIP_CONFIG_SECURITY_TEST_MODE 0

#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8005
#define CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION 1
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_REVISION 1

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "0.1ALPHA"
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 0x0001
#endif

#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "TEST_SN"

#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (512)

#define CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY 1

#define CHIP_BLE_DEVICE_NAME "ContactSensor"
