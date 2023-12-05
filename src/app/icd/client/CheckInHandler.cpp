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
 *      This file defines objects for a CHIP ICD handler which handles unsolicited checkin messages.
 *
 */

#include "CheckInHandler.h"
#include "CheckInDelegate.h"

#include <cinttypes>

#include <lib/core/Global.h>
#include <lib/support/CodeUtils.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>

#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {

static Global<CheckInMessageHandler> sCheckInMessageHandler;
CheckInMessageHandler * CheckInMessageHandler::GetInstance()
{
    return &sCheckInMessageHandler.get();
}

CHIP_ERROR CheckInMessageHandler::Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage)
{
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(clientStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mExchangeManager  = exchangeManager;
    mICDClientStorage = static_cast<DefaultICDClientStorage *>(clientStorage);
    ReturnErrorOnFailure(
        exchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn, this));

    return CHIP_NO_ERROR;
}

void CheckInMessageHandler::Shutdown()
{
    if (mExchangeManager)
    {
        mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn);
        mExchangeManager = nullptr;
    }
}
CHIP_ERROR CheckInMessageHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
{
    // Return error for wrong message type
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn), CHIP_ERROR_INVALID_ARGUMENT);

    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckInMessageHandler::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                    System::PacketBufferHandle && payload)
{
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn), CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan payloadByteSpan{ payload->Start(), payload->DataLength() };
    auto * iterator = mICDClientStorage->IterateICDClientInfo();
    CHIP_ERROR err  = CHIP_NO_ERROR;
    ICDClientInfo clientInfo;
    while (iterator->Next(clientInfo))
    {
        err = mICDClientStorage->ProcessCheckInPayload(payloadByteSpan, clientInfo);
        if (err == CHIP_NO_ERROR)
        {
            OnCheckInComplete();
            return err;
        }
    }
    return err;
}

void CheckInMessageHandler::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

} // namespace app
} // namespace chip
