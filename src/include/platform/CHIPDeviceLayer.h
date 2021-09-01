/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <platform/CHIPDeviceConfig.h>

#if !CHIP_DEVICE_LAYER_NONE

#include <ble/BleLayer.h>
#include <lib/core/CHIPCore.h>
#include <platform/CHIPDeviceError.h>
#include <platform/ConfigurationManager.h>
#include <platform/ConnectivityManager.h>
#include <platform/GeneralUtils.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>
#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#include <platform/SoftwareUpdateManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if CHIP_DEVICE_CONFIG_ENABLE_NFC
#include <platform/NFCManager.h>
#endif

namespace chip {
namespace DeviceLayer {

struct ChipDeviceEvent;
extern chip::System::Layer SystemLayer;
extern Inet::InetLayer InetLayer;

} // namespace DeviceLayer
} // namespace chip

#endif // !CHIP_DEVICE_LAYER_NONE
