/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file wifi_config.h
 * @brief Matter SiWx Wi-Fi device configuration.
 *
 * Configuration is built from a default value, then optional parameters are applied:
 *
 *   config = MatterWifiGetDefaultDeviceConfiguration();
 *   MatterWifiApplyOptionalDeviceConfiguration(&config);
 *
 * Include ble_config.h before this header.
 */

#pragma once

#include "sl_status.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_device.h"

#ifndef REGION_CODE
#define REGION_CODE US
#endif // !REGION_CODE

#if (SL_SI91X_ACX_MODULE == 1)
#define REGION_CODE_BITMAP IGNORE_REGION
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH
#else
#define REGION_CODE_BITMAP REGION_CODE
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif // SL_SI91X_ACX_MODULE

#if (USE_BYPASS_CLOCK == 1)
#define SL_SI91X_CLK SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(2)
#else
#define SL_SI91X_CLK SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(1)
#endif // USE_BYPASS_CLOCK

#ifdef __cplusplus
extern "C" {
#endif

sl_wifi_device_configuration_t MatterWifiGetDefaultDeviceConfiguration(void);
void MatterWifiApplyOptionalDeviceConfiguration(sl_wifi_device_configuration_t * configuration);

#ifdef __cplusplus
}
#endif
