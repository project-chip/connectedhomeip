/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *      This file defines objects for a CHIP ICD handler which handles unsolicited Check-In messages.
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

inline constexpr uint64_t kCheckInCounterMax = (1ULL << 32);
inline constexpr uint32_t kKeyRefreshLimit   = (1U << 31);

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

    return mpExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn, this);
}

void CheckInMessageHandler::Shutdown()
{
    mpICDClientStorage = nullptr;
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
    // If the message type is not ICD_CheckIn, return CHIP_NO_ERROR and exit
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn), CHIP_NO_ERROR);

    ByteSpan payloadByteSpan{ payload->Start(), payload->DataLength() };
    ICDClientInfo clientInfo;
    CounterType counter = 0;
    // If the CheckIn message processing fails, return CHIP_NO_ERROR and exit.
    VerifyOrReturnError(CHIP_NO_ERROR == mpICDClientStorage->ProcessCheckInPayload(payloadByteSpan, clientInfo, counter),
                        CHIP_NO_ERROR);
    CounterType checkInCounter = (counter - clientInfo.start_icd_counter) % kCheckInCounterMax;
    VerifyOrReturnError(checkInCounter > clientInfo.offset, CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
    clientInfo.offset = checkInCounter;
    bool refreshKey   = (checkInCounter > kKeyRefreshLimit);
    ByteSpan newKeyData;
    if (refreshKey)
    {
        mpCheckInDelegate->OnRefreshKey(newKeyData);
        RegisterClientWithNewKey(clientInfo, newKeyData);
    }
    mpCheckInDelegate->OnCheckInComplete(clientInfo);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckInMessageHandler::RegisterClientWithNewKey(ICDClientInfo & clientInfo, const ByteSpan keyData)
{
    // TODO - Register the client. On successful registration, update the clientInfo with the new key and store the clientInfo
    return CHIP_NO_ERROR;
}
void CheckInMessageHandler::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

} // namespace app
} // namespace chip
