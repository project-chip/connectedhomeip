/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Example project configuration file for CHIP.
 *
 *          This is a place to put application or project-specific overrides
 *          to the default configuration values for general CHIP features.
 *
 */

#pragma once
#define CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT 100
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "Rang Dong"
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "Home Center"
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION_STRING "1.0"
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION 1
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING "1.0"
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION 1
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER "123456789"
#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 22

#define CHIP_DEVICE_CONFIG_ENABLE_WIFI 1

#define LOG_HEAD "RANG DONG DEV - "
// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>
