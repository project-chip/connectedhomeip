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
 *      This file defines objects for a CHIP ICD handler which handles unsolicited check-in messages.
 *
 */

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelTimeout.h>
#include <app/icd/client/CheckInHandler.h>
#include <app/icd/client/RefreshKeySender.h>

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

CheckInHandler::CheckInHandler() {}

CHIP_ERROR CheckInHandler::Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage,
                                CheckInDelegate * delegate, InteractionModelEngine * engine)
{
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(clientStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpExchangeManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpICDClientStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);

    mpExchangeManager  = exchangeManager;
    mpICDClientStorage = clientStorage;
    mpCheckInDelegate  = delegate;
    mpImEngine         = engine;
    return mpExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn, this);
}

void CheckInHandler::Shutdown()
{
    mpICDClientStorage = nullptr;
    mpCheckInDelegate  = nullptr;
    if (mpExchangeManager)
    {
        mpExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::ICD_CheckIn);
        mpExchangeManager = nullptr;
    }
}

CHIP_ERROR CheckInHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
{
    // Return error for wrong message type
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckInHandler::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                             System::PacketBufferHandle && payload)
{
    // If the message type is not ICD_CheckIn, return CHIP_NO_ERROR and exit
    VerifyOrReturnError(payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::ICD_CheckIn), CHIP_NO_ERROR);

    ByteSpan payloadByteSpan{ payload->Start(), payload->DataLength() };
    ICDClientInfo clientInfo;
    CounterType counter = 0;
    // If the check-in message processing fails, return CHIP_NO_ERROR and exit.
    CHIP_ERROR err = mpICDClientStorage->ProcessCheckInPayload(payloadByteSpan, clientInfo, counter);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(ICD, "ProcessCheckInPayload failed: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_NO_ERROR;
    }
    CounterType receivedCheckInCounterOffset = (counter - clientInfo.start_icd_counter) % kCheckInCounterMax;

    // Detect duplicate check-in messages and return CHIP_NO_ERROR on receiving a duplicate message
    if (receivedCheckInCounterOffset <= clientInfo.offset)
    {
        ChipLogError(ICD, "A duplicate check-in message was received and discarded");
        return CHIP_NO_ERROR;
    }

    clientInfo.offset = receivedCheckInCounterOffset;
    bool refreshKey   = (receivedCheckInCounterOffset > kKeyRefreshLimit);

    if (refreshKey)
    {
        RefreshKeySender * refreshKeySender = mpCheckInDelegate->OnKeyRefreshNeeded(clientInfo, mpICDClientStorage);
        if (refreshKeySender == nullptr)
        {
            ChipLogError(ICD, "Key Refresh failed for node ID:" ChipLogFormatScopedNodeId,
                         ChipLogValueScopedNodeId(clientInfo.peer_node));
            return CHIP_NO_ERROR;
        }
        err = refreshKeySender->EstablishSessionToPeer();
        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(ICD, "CASE session establishment failed with error : %" CHIP_ERROR_FORMAT, err.Format());
            mpCheckInDelegate->OnKeyRefreshDone(refreshKeySender, err);
            return CHIP_NO_ERROR;
        }
    }
    else
    {
        mpCheckInDelegate->OnCheckInComplete(clientInfo);
#if CHIP_CONFIG_ENABLE_READ_CLIENT
        mpImEngine->OnActiveModeNotification(clientInfo.peer_node);
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT
    }

    return CHIP_NO_ERROR;
}

void CheckInHandler::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

} // namespace app
} // namespace chip
