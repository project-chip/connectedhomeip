/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file implements the CHIP Connection object that maintains an NFC connection.
 *
 */

#include <transport/raw/NFC.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <platform/internal/NFCCommissioningManager.h>

#include <inttypes.h>

#include <lib/support/BytesToHex.h>

using namespace chip::System;

namespace chip {
namespace Transport {

NFCBase::~NFCBase()
{
    ClearState();
}

void NFCBase::ClearState()
{
    mState = State::kNotReady;
}

CHIP_ERROR NFCBase::Init(const NfcListenParameters & params)
{
    ChipLogProgress(Controller, "NFCBase::Init");

    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

    mState = State::kInitialized;

    chip::DeviceLayer::Internal::NFCCommissioningMgrImpl().SetNFCBase(this);

    return CHIP_NO_ERROR;
}

// Check if it is possible to communicate with this Peer
bool NFCBase::CanSendToPeer(const Transport::PeerAddress & address)
{
    if (mState == State::kNotReady)
    {
        ChipLogError(Controller, "Invalid state in NFCBase::CanSendToPeer()");
        return false;
    }

    bool canSendToPeer = DeviceLayer::Internal::NFCCommissioningMgrImpl().CanSendToPeer(address);
    if (canSendToPeer)
    {
        mState = State::kConnected;
    }
    else
    {
        mState = State::kInitialized;
    }

    return canSendToPeer;
}

void NFCBase::OnNfcTagResponse(const Transport::PeerAddress & address, System::PacketBufferHandle && buffer)
{
    ChipLogProgress(Controller, "NFCBase::OnNfcTagResponse");

    HandleMessageReceived(address, std::move(buffer));
}

void NFCBase::OnNfcTagError(const Transport::PeerAddress & address)
{
    ChipLogProgress(Controller, "NFCBase::OnNfcTagError");
}

CHIP_ERROR NFCBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kNfc, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState == State::kConnected, CHIP_ERROR_INCORRECT_STATE);

    DeviceLayer::Internal::NFCCommissioningMgrImpl().SendToNfcTag(address, std::move(msgBuf));

    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
