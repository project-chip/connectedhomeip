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
#include <platform/KeyValueStoreManager.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>
#include <system/SystemLayerImpl.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if CHIP_DEVICE_CONFIG_ENABLE_NFC
#include <platform/NFCManager.h>
#endif

namespace chip {
namespace DeviceLayer {

namespace Internal {

extern const char * const TAG;

// These globals should only be used via the functions declared below.
extern System::Clock::ClockBase * gClock;
extern System::Layer * gSystemLayer;
extern Inet::EndPointManager<Inet::UDPEndPoint> * gUDPEndPointManager;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
extern Inet::EndPointManager<Inet::TCPEndPoint> * gTCPEndPointManager;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

// Inside the Internal namespace to emphasize that it is not for general use.
inline void SetSystemLayerForTesting(System::Layer * layer)
{
    Internal::gSystemLayer = layer;
}

}; // namespace Internal

/**
 * Access to the global System::Layer singleton.
 */
inline System::Layer & SystemLayer()
{
    return *Internal::gSystemLayer;
}

/**
 * Access to the global EndPointManager<UDPEndPoint> singleton.
 */
inline Inet::EndPointManager<Inet::UDPEndPoint> * UDPEndPointManager()
{
    return Internal::gUDPEndPointManager;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT

/**
 * Access to the global EndPointManager<TCPEndPoint> singleton.
 *
 * Only valid after `InitTCPEndPointManager()` has been called.
 */
inline Inet::EndPointManager<Inet::TCPEndPoint> * TCPEndPointManager()
{
    return Internal::gTCPEndPointManager;
}

/**
 * Initialize the global EndPointManager<TCPEndPoint> singleton.
 *
 * Must be called before any use of TCP (i.e., any call to `TCPEndPointManager()`).
 * Must be paired with a call to `TCPEndPointManager()->Shutdown()` on exit, after any
 * TCP EndPoints have been released and before `SystemLayer().Shutdown()`.
 */
CHIP_ERROR InitTCPEndPointManager();

#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

} // namespace DeviceLayer
} // namespace chip

#endif // !CHIP_DEVICE_LAYER_NONE
