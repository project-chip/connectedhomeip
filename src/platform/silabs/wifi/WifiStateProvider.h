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
#pragma once

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

/**
 * @brief Wi-Fi state provider interface
 *
 */
class WifiStateProvider
{
public:
    virtual ~WifiStateProvider() = default;

    /**
     * @brief Returns the state of the Wi-Fi connection
     *
     * @return true, if the Wi-Fi device is connected to an AP
     *         false, otherwise
     */
    virtual bool IsStationConnected() = 0;

    /**
     * @brief Returns the state of the Wi-Fi Station configuration of the Wi-Fi device
     *
     * @return true, if the Wi-Fi Station mode is enabled
     *         false, otherwise
     */
    virtual bool IsStationModeEnabled() = 0;

    /**
     * @brief Returns the state of the Wi-Fi station initialization
     *
     * @return true, if the initialization was successful
     *         false, otherwise
     */
    virtual bool IsStationReady() = 0;

    /**
     * @brief Returns the state of the Wi-Fi network provisionning
     *        Does the device has Wi-Fi credentials or not
     *
     * @return true, the device has Wi-Fi credentials
     *         false, otherwise
     */
    virtual bool IsWifiProvisioned() = 0;

    /**
     * @brief Returns IP assignment status
     *

     * @return true, Wi-Fi station has an IPv4 address
     *         false, otherwise
     */
    virtual bool HasAnIPv4Address() = 0;

    /**
     * @brief Returns IP assignment status
     *

     * @return true, Wi-Fi station has an IPv6 address
     *         false, otherwise
     */
    virtual bool HasAnIPv6Address() = 0;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
