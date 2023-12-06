/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/CheckInHandler.h>

#include <cinttypes>

#include <lib/core/Global.h>
#include <lib/support/CodeUtils.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>

#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {

CHIP_ERROR CheckInMessageHandler::Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage,
                                       CheckInDelegate * delegate)
{
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(clientStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpExchangeManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpICDClientStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpExchangeManager  = exchangeManager;
    mpICDClientStorage = clientStorage;
    mpCheckInDelegate  = delegate;
    ReturnErrorOnFailure(
        mpExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn, this));

    return CHIP_NO_ERROR;
}

void CheckInMessageHandler::Shutdown()
{
    if (mpExchangeManager)
    {
        mpExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn);
        mpExchangeManager = nullptr;
    }
}
CHIP_ERROR CheckInMessageHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
{
    // Return error for wrong message type
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckInMessageHandler::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                    System::PacketBufferHandle && payload)
{
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    ByteSpan payloadByteSpan{ payload->Start(), payload->DataLength() };
    ICDClientInfo clientInfo;
    bool needRefreshKey = false;
    VerifyOrReturnError(mICDClientStorage->ProcessCheckInPayload(payloadByteSpan, clientInfo, needRefreshKey),
                        CHIP_ERROR_INCORRECT_STATE);
    mCheckInDelegate->OnCheckInComplete(clientInfo, needRefreshKey);
    return CHIP_NO_ERROR;
}

void CheckInMessageHandler::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

} // namespace app
} // namespace chip
