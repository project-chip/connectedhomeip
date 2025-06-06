/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 144 // 0x0090 Network Infrastructure Manager
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Network Infrastructure Manager"
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8013

// Sufficient space for ArlReviewEvent of several fabrics.
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE (32 * 1024)

// Inherit defaults from config/standalone/CHIPProjectConfig.h
#include <CHIPProjectConfig.h>
