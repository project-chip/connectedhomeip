/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *      This file implements the Matter Connection object that maintains a Wi-Fi PAF connection
 *
 */

#include <inttypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/WiFiPAF.h>

using namespace chip::System;

namespace chip {
namespace Transport {

WiFiPAFBase::~WiFiPAFBase()
{
    ClearState();
}

void WiFiPAFBase::ClearState()
{
    mState = State::kNotReady;
}

CHIP_ERROR WiFiPAFBase::Init(const WiFiPAFListenParameters & param)
{
    ChipLogDetail(Inet, "WiFiPAFBase::Init - setting/overriding transport");
    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    DeviceLayer::ConnectivityMgr().SetWiFiPAF(this);
    mState = State::kInitialized;

    if (!DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
    {
        ChipLogError(Inet, "Wi-Fi Management has not started, do it now.");
        static constexpr useconds_t kWiFiStartCheckTimeUsec = WIFI_START_CHECK_TIME_USEC;
        static constexpr uint8_t kWiFiStartCheckAttempts    = WIFI_START_CHECK_ATTEMPTS;
        DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
        {
            for (int cnt = 0; cnt < kWiFiStartCheckAttempts; cnt++)
            {
                if (DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
                {
                    break;
                }
                usleep(kWiFiStartCheckTimeUsec);
            }
        }
        if (!DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
        {
            ChipLogError(Inet, "Wi-Fi Management taking too long to start - device configuration will be reset.");
            return CHIP_ERROR_INTERNAL;
        }
        ChipLogProgress(NotSpecified, "Wi-Fi Management is started");
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    ReturnErrorCodeIf(address.GetTransportType() != Type::kWiFiPAF, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    DeviceLayer::ConnectivityMgr().WiFiPAFSend(std::move(msgBuf));

    return CHIP_NO_ERROR;
}

void WiFiPAFBase::OnWiFiPAFMessageReceived(System::PacketBufferHandle && buffer)
{
    HandleMessageReceived(Transport::PeerAddress(Transport::Type::kWiFiPAF), std::move(buffer));
    return;
}

CHIP_ERROR WiFiPAFBase::SendAfterConnect(System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].IsNull())
        {
            mPendingPackets[i] = std::move(msg);
            err                = CHIP_NO_ERROR;
            break;
        }
    }

    return err;
}

} // namespace Transport
} // namespace chip
