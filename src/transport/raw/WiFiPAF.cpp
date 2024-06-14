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
 *      This file implements the CHIP Connection object that maintains a BLE connection.
 *
 */

//#include <transport/raw/BLE.h>
#include <transport/raw/WiFiPAF.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>
#include <platform/ConnectivityManager.h>

#include <inttypes.h>

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

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    ChipLogProgress(NotSpecified, "=====> WiFiPAFBase::SendMessage()");
    ReturnErrorCodeIf(address.GetTransportType() != Type::kWiFiPAF, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

    DeviceLayer::ConnectivityMgr().WiFiPAFSend(std::move(msgBuf));

    ChipLogProgress(NotSpecified, "<===== WiFiPAFBase::SendMessage()");
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
