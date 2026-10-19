/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 * @file SlNetConfig.h
 * @brief Matter SiWx Wi-Fi device configuration.
 */

#pragma once

#include "sl_status.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_device.h"

#if (SL_SI91X_ACX_MODULE == 1)
#define REGION_CODE_BITMAP IGNORE_REGION
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH
#endif // SL_SI91X_ACX_MODULE

#if (USE_BYPASS_CLOCK == 1)
#define SL_SI91X_CLK SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(2)
#endif // USE_BYPASS_CLOCK

#ifndef REGION_CODE_BITMAP
#define REGION_CODE_BITMAP US
#endif // !REGION_CODE_BITMAP

#ifndef FRONT_END_SWITCH_CTRL
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif // !FRONT_END_SWITCH_CTRL

#ifndef SL_SI91X_CLK
#define SL_SI91X_CLK SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(1)
#endif // !SL_SI91X_CLK

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the SiWx Wi-Fi device configuration for the active build.
 *
 * Builds the full @c sl_wifi_device_configuration_t used to bring up the device, including
 * Wi-Fi options (NCP GPIO handshake, IPv6, max RX length) and BLE coexistence when enabled.
 *
 * @return sl_wifi_device_configuration_t The device configuration by value.
 */
sl_wifi_device_configuration_t SLNetGetDeviceConfig(void);

#ifdef __cplusplus
}
#endif
