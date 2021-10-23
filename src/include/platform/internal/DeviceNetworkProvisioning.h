/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the CHIP Device Network Provisioning object.
 *
 */

#pragma once

#include <lib/core/CHIPCore.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class DLL_EXPORT DeviceNetworkProvisioningDelegate
{
public:
    /**
     * @brief
     *   Called to provision WiFi credentials in a device
     *
     * @param ssid WiFi SSID
     * @param passwd WiFi password
     */
    virtual CHIP_ERROR ProvisionWiFi(const char * ssid, const char * passwd) = 0;

    /**
     * @brief
     *   Called to provision Thread credentials in a device
     *
     */
    virtual CHIP_ERROR ProvisionThread(ByteSpan threadData) = 0;

    virtual ~DeviceNetworkProvisioningDelegate() {}
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
