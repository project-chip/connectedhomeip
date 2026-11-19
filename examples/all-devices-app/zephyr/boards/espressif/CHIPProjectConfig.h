/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 * ESP32 project configuration, selected through CONFIG_CHIP_PROJECT_CONFIG in
 * socs/esp32*.conf. Buffer sizes are reduced for the ESP32 SRAM budget, as in
 * examples/lighting-app/esp32_zephyr.
 */

#pragma once

#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 8
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE (256)
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE (256)

#include "../../CHIPProjectConfig.h"
