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
namespace Controller {

void ScriptDevicePairingDelegate::SetWifiCredential(const char * ssid, const char * password)
{
    strncpy(mWifiSSID, ssid, sizeof(mWifiSSID));
    strncpy(mWifiPassword, password, sizeof(mWifiPassword));
    mMode = Mode::Wifi;
}

void ScriptDevicePairingDelegate::SetThreadCredential(uint8_t channel, uint16_t panId,
                                                      uint8_t (&masterKey)[chip::DeviceLayer::Internal::kThreadMasterKeyLength])
{
    mThreadInfo               = {};
    mThreadInfo.ThreadChannel = channel;
    mThreadInfo.ThreadPANId   = panId;
    memcpy(mThreadInfo.ThreadMasterKey, masterKey, sizeof(masterKey));
    mMode = Mode::Thread;
}

void ScriptDevicePairingDelegate::OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback)
{
    if (mMode == Mode::Wifi)
        callback->SendNetworkCredentials(mWifiSSID, mWifiPassword);
    else
        callback->SendThreadCredentials(mThreadInfo);
}

void ScriptDevicePairingDelegate::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                                    RendezvousDeviceCredentialsDelegate * callback)
{
    // TODO: Implement this
    ChipLogDetail(Controller, "ScriptDevicePairingDelegate::OnOperationalCredentialsRequested\n");
}

void ScriptDevicePairingDelegate::SetKeyExchangeCallback(DevicePairingDelegate_OnPairingCompleteFunct callback)
{
    mOnPairingCompleteCallback = callback;
}

void ScriptDevicePairingDelegate::OnPairingComplete(CHIP_ERROR error)
{
    if (mOnPairingCompleteCallback != nullptr)
    {
        mOnPairingCompleteCallback(error);
    }
}

} // namespace Controller
} // namespace chip
