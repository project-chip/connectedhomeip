/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// include the CHIPProjectConfig from config/standalone
#include <CHIPProjectConfig.h>

// Advertise the device type and name during commissioning. Both are gated: without
// these, CHIP_DEVICE_CONFIG_DEVICE_TYPE and _DEVICE_NAME below have no effect,
// because Dnssd.cpp checks IsCommissionableDeviceTypeEnabled() /
// IsCommissionableDeviceNameEnabled() before emitting the DT and DN TXT keys.
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_TYPE 1
#define CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONABLE_DEVICE_NAME 1

#define CHIP_DEVICE_CONFIG_DEVICE_TYPE 0x0517 // Electrical Distribution Enclosure
// Must fit chip::Dnssd::kKeyDeviceNameMaxLength (32) so the commissionable "DN"
// TXT key is advertised; the full device type name would be 33 characters.
#define CHIP_DEVICE_CONFIG_DEVICE_NAME "Electrical Protection"
