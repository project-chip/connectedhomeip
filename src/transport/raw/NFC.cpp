/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

//using namespace chip::Nfc;
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

void NFCBase::OnNfcTagResponse(System::PacketBufferHandle && buffer)
{
    ChipLogProgress(Controller, "NFCBase::OnNfcTagResponse");

#ifdef DUMP_BLE_MESSAGES
    chip::Encoding::LogByteArrayAsHex("NFCBase::OnNfcTagResponse", buffer->Start(), buffer->DataLength());
#endif

    HandleMessageReceived(Transport::PeerAddress(Transport::Type::kNfc), std::move(buffer));
}

void NFCBase::OnNfcTagError()
{
    ChipLogProgress(Controller, "NFCBase::OnNfcTagError");
}

CHIP_ERROR NFCBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
#ifdef DUMP_BLE_MESSAGES
    chip::Encoding::LogByteArrayAsHex("NFCBase::SendMessage()", msgBuf->Start(), msgBuf->DataLength());
#endif

    VerifyOrReturnError(address.GetTransportType() == Type::kNfc, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState != State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

    chip::DeviceLayer::Internal::NFCCommissioningMgrImpl().SendToNfcTag(std::move(msgBuf));

    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
