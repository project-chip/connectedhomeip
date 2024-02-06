/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/icd/server/ICDCheckInSender.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDNotifier.h>
#include <lib/dnssd/Resolver.h>
#include <protocols/secure_channel/CheckinMessage.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

using namespace Protocols::SecureChannel;

ICDCheckInSender::ICDCheckInSender(Messaging::ExchangeManager * exchangeManager)
{
    VerifyOrDie(exchangeManager != nullptr);
    mExchangeManager = exchangeManager;
    mAddressLookupHandle.SetListener(this);
}

void ICDCheckInSender::OnNodeAddressResolved(const PeerId & peerId, const AddressResolve::ResolveResult & result)
{
    if (CHIP_NO_ERROR != SendCheckInMsg(result.address))
    {
        ChipLogError(AppServer, "Failed to send the ICD Check-In message");
    }

    ICDNotifier::GetInstance().NotifyActiveRequestWithdrawal(ICDListener::KeepActiveFlag::kCheckInInProgress);
}

void ICDCheckInSender::OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason)
{
    ICDNotifier::GetInstance().NotifyActiveRequestWithdrawal(ICDListener::KeepActiveFlag::kCheckInInProgress);
    ChipLogProgress(AppServer, "Node Address resolution failed for ICD Check-In with Node ID " ChipLogFormatX64,
                    ChipLogValueX64(peerId.GetNodeId()));
}

CHIP_ERROR ICDCheckInSender::SendCheckInMsg(const Transport::PeerAddress & addr)
{
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(CheckinMessage::kMinPayloadSize);

    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };

    // Encoded ActiveModeThreshold in littleEndian for Check-In message application data
    {
        uint8_t activeModeThresholdBuffer[kApplicationDataSize] = { 0 };
        size_t writtenBytes                                     = 0;
        Encoding::LittleEndian::BufferWriter writer(activeModeThresholdBuffer, sizeof(activeModeThresholdBuffer));

        uint16_t activeModeThreshold_ms = ICDConfigurationData::GetInstance().GetActiveModeThreshold().count();
        writer.Put16(activeModeThreshold_ms);
        VerifyOrReturnError(writer.Fit(writtenBytes), CHIP_ERROR_INTERNAL);

        ByteSpan activeModeThresholdByteSpan(writer.Buffer(), writtenBytes);

        ReturnErrorOnFailure(CheckinMessage::GenerateCheckinMessagePayload(mAes128KeyHandle, mHmac128KeyHandle, mICDCounter,
                                                                           activeModeThresholdByteSpan, output));
    }

    buffer->SetDataLength(static_cast<uint16_t>(output.size()));

    VerifyOrReturnError(mExchangeManager->GetSessionManager() != nullptr, CHIP_ERROR_INTERNAL);

    // Using default MRP since we are not doing MRP in this context
    Optional<SessionHandle> session =
        mExchangeManager->GetSessionManager()->CreateUnauthenticatedSession(addr, GetDefaultMRPConfig());
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);

    Messaging::ExchangeContext * exchangeContext = mExchangeManager->NewContext(session.Value(), nullptr);

    VerifyOrReturnError(exchangeContext != nullptr, CHIP_ERROR_NO_MEMORY);

    return exchangeContext->SendMessage(MsgType::ICD_CheckIn, std::move(buffer), Messaging::SendMessageFlags::kNoAutoRequestAck);
}

CHIP_ERROR ICDCheckInSender::RequestResolve(ICDMonitoringEntry & entry, FabricTable * fabricTable, uint32_t counter)
{
    VerifyOrReturnError(entry.IsValid(), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fabricTable != nullptr, CHIP_ERROR_INTERNAL);
    const FabricInfo * fabricInfo = fabricTable->FindFabricWithIndex(entry.fabricIndex);
    PeerId peerId(fabricInfo->GetCompressedFabricId(), entry.checkInNodeID);

    mICDCounter = counter;

    AddressResolve::NodeLookupRequest request(peerId);

    memcpy(mAes128KeyHandle.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
           entry.aesKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(), sizeof(Crypto::Symmetric128BitsKeyByteArray));

    memcpy(mHmac128KeyHandle.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
           entry.hmacKeyHandle.As<Crypto::Symmetric128BitsKeyByteArray>(), sizeof(Crypto::Symmetric128BitsKeyByteArray));

    CHIP_ERROR err = AddressResolve::Resolver::Instance().LookupNode(request, mAddressLookupHandle);

    if (err == CHIP_NO_ERROR)
    {
        ICDNotifier::GetInstance().NotifyActiveRequestNotification(ICDListener::KeepActiveFlag::kCheckInInProgress);
    }

    return err;
}

} // namespace app
} // namespace chip
