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

#include <app/CommandPathParams.h>
#include <app/InteractionModelTimeout.h>
#include <app/icd/client/CheckInDelegate.h>
#include <app/icd/client/CheckInHandler.h>
#include <app/icd/client/ICDRefreshKeyInfo.h>

#include <cinttypes>

#include <lib/core/Global.h>
#include <lib/support/CodeUtils.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>

#include <app/InteractionModelEngine.h>
#include <app/OperationalSessionSetup.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {

inline constexpr uint64_t kCheckInCounterMax = (1ULL << 32);
inline constexpr uint32_t kKeyRefreshLimit   = (1U << 31);

CheckInHandler::CheckInHandler() :
    mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{}

CHIP_ERROR CheckInHandler::Init(Messaging::ExchangeManager * exchangeManager, ICDClientStorage * clientStorage,
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
    VerifyOrReturnError(CHIP_NO_ERROR == mpICDClientStorage->ProcessCheckInPayload(payloadByteSpan, clientInfo, counter),
                        CHIP_NO_ERROR);
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
        uint8_t newKeyData[chip::Crypto::kAES_CCM128_Key_Length];
        mpCheckInDelegate->OnRefreshKeyGenerate(clientInfo, newKeyData, chip::Crypto::kAES_CCM128_Key_Length);
        // A new session should be established to re-register the client using the new key. The registration will happen in
        // mOnDeviceConnected callback
        EstablishSessionToPeer(clientInfo.peer_node);
    }
    else
    {
        mpCheckInDelegate->OnCheckInComplete(clientInfo);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckInHandler::RegisterClientWithNewKey(ICDClientInfo & clientInfo, ByteSpan newKey,
                                                    Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    // using namespace Clusters::IcdManagement;
    // TODO : Determine if using an Objectpool of commandSenders is the best approach here
    //  app::CommandSender registerCommandSender(&registerCommandSenderDelegate, &exchangeMgr);

    // auto commandPathParams = CommandPathParams(0, 0, Id, Commands::RegisterClient::Id, (CommandPathFlags::kEndpointIdValid));
    // ReturnErrorOnFailure(registerCommandSender.PrepareCommand(commandPathParams));

    // chip::TLV::TLVWriter * writer = registerCommandSender.GetCommandDataIBTLVWriter();

    // ReturnErrorOnFailure(
    //     writer->Put(chip::TLV::ContextTag(Commands::RegisterClient::Fields::kCheckInNodeID), clientInfo.peer_node.GetNodeId()));
    // ReturnErrorOnFailure(
    //     writer->Put(chip::TLV::ContextTag(Commands::RegisterClient::Fields::kMonitoredSubject), clientInfo.monitored_subject));
    // ReturnErrorOnFailure(writer->Put(chip::TLV::ContextTag(Commands::RegisterClient::Fields::kKey), newKey));

    // // TODO : We don't have plain data for the old key

    // ReturnErrorOnFailure(registerCommandSender.FinishCommand());
    // ReturnErrorOnFailure(registerCommandSender.SendCommandRequest(sessionHandle));

    return CHIP_NO_ERROR;
}

void CheckInHandler::EstablishSessionToPeer(ScopedNodeId peerId)
{
    ChipLogProgress(ICD, "Trying to establish a CASE session for re-registering an ICD client");
    auto * caseSessionManager = InteractionModelEngine::GetInstance()->GetCASESessionManager();
    VerifyOrReturn(caseSessionManager != nullptr);
    caseSessionManager->FindOrEstablishSession(peerId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
}

void CheckInHandler::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                           const SessionHandle & sessionHandle)
{
    CheckInHandler * const _this = static_cast<CheckInHandler *>(context);
    VerifyOrDie(_this != nullptr);
    ICDRefreshKeyInfo refreshKeyInfo;
    if (CHIP_NO_ERROR !=
        _this->mpCheckInDelegate->OnRefreshKeyRetrieve(sessionHandle->AsSecureSession()->GetPeer(), refreshKeyInfo))
    {
        ChipLogError(ICD, "Failed to retrieve a new key for re-registration of the ICD client");
    }
    ByteSpan newKey(refreshKeyInfo.newKey);
    if (CHIP_NO_ERROR != _this->RegisterClientWithNewKey(refreshKeyInfo.clientInfo, newKey, exchangeMgr, sessionHandle))
    {
        ChipLogError(ICD, "Failed to send register client command");
    }
}

void CheckInHandler::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    CheckInHandler * const _this = static_cast<CheckInHandler *>(context);
    VerifyOrDie(_this != nullptr);

    ChipLogError(ICD, "Failed to establish CASE for re-registration with error '%" CHIP_ERROR_FORMAT "'", err.Format());
}

void CheckInHandler::OnResponseTimeout(Messaging::ExchangeContext * ec) {}

} // namespace app
} // namespace chip
