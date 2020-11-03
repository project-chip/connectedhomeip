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

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"

#include <transport/RendezvousSessionDelegate.h>

namespace chip {
namespace DeviceController {

void ScriptDevicePairingDelegate::SetWifiCredential(const char * ssid, const char * password)
{
    strncpy(mWifiSSID, ssid, sizeof(mWifiSSID));
    strncpy(mWifiPassword, password, sizeof(mWifiPassword));
}

void ScriptDevicePairingDelegate::OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback)
{
    callback->SendNetworkCredentials(mWifiSSID, mWifiPassword);
}

void ScriptDevicePairingDelegate::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                                    RendezvousDeviceCredentialsDelegate * callback)
{
    // TODO: Implement this
    ChipLogDetail(Controller, "ScriptDevicePairingDelegate::OnOperationalCredentialsRequested\n");
}

} // namespace DeviceController
} // namespace chip
