/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/CHIPDeviceConfig.h>

#if !CHIP_DEVICE_LAYER_NONE

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

void SetSystemLayerForTesting(System::Layer * layer);

// These functions are defined in src/platform/Globals.cpp
chip::System::Layer & SystemLayer();

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
chip::System::LayerSockets & SystemLayerSockets();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

inline chip::Inet::EndPointManager<Inet::UDPEndPoint> * UDPEndPointManager()
{
    return &ConnectivityMgr().UDPEndPointManager();
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
inline chip::Inet::EndPointManager<Inet::TCPEndPoint> * TCPEndPointManager()
{
    return &ConnectivityMgr().TCPEndPointManager();
}
#endif

} // namespace DeviceLayer
} // namespace chip

#endif // !CHIP_DEVICE_LAYER_NONE
