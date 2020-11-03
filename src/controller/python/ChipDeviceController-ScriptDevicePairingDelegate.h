/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      Declaration of DevicePairingDelegate of CHIP Device Controller for Python
 *
 */

#pragma once

#include <controller/CHIPDeviceController_deprecated.h>

#include <platform/internal/DeviceNetworkInfo.h>
#include <transport/RendezvousSessionDelegate.h>

namespace chip {
namespace DeviceController {

class ScriptDevicePairingDelegate final : public Controller::DevicePairingDelegate
{
public:
    ~ScriptDevicePairingDelegate() = default;
    void SetWifiCredential(const char * ssid, const char * password);
    void OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback) override;

    void OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                           RendezvousDeviceCredentialsDelegate * callback) override;

private:
    // WiFi Provisioning Data
    char mWifiSSID[chip::DeviceLayer::Internal::kMaxWiFiSSIDLength + 1];
    char mWifiPassword[chip::DeviceLayer::Internal::kMaxWiFiKeyLength];
};

} // namespace DeviceController
} // namespace chip
