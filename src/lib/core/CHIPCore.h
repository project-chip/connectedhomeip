/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file is the master "umbrella" include file for the core
 *      chip library.
 *
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#include <system/SystemLayer.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif // CONFIG_NETWORK_LAYER_BLE

#define CHIP_CORE_IDENTITY "chip-core"
#define CHIP_CORE_PREFIX CHIP_CORE_IDENTITY ": "

#include <lib/core/CHIPError.h>
