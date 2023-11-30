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

#include "ICDCheckInSender.h"

#include "ICDNotifier.h"

#include <system/SystemPacketBuffer.h>

#include <protocols/secure_channel/CheckinMessage.h>

#include <lib/dnssd/Resolver.h>

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

    ICDNotifier::GetInstance().BroadcastActiveRequestWithdrawal(ICDListener::KeepActiveFlag::kCheckInInProgress);
}

void ICDCheckInSender::OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason)
{
    ICDNotifier::GetInstance().BroadcastActiveRequestWithdrawal(ICDListener::KeepActiveFlag::kCheckInInProgress);
    ChipLogProgress(AppServer, "Node Address resolution failed for ICD Check-In with Node ID " ChipLogFormatX64,
                    ChipLogValueX64(peerId.GetNodeId()));
}

CHIP_ERROR ICDCheckInSender::SendCheckInMsg(const Transport::PeerAddress & addr)
{
    System::PacketBufferHandle buffer = MessagePacketBuffer::New(CheckinMessage::sMinPayloadSize);

    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan output{ buffer->Start(), buffer->MaxDataLength() };

    ReturnErrorOnFailure(CheckinMessage::GenerateCheckinMessagePayload(mKey, mICDCounter, ByteSpan(), output));
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

    memcpy(mKey.AsMutable<Crypto::Aes128KeyByteArray>(), entry.key.As<Crypto::Aes128KeyByteArray>(),
           sizeof(Crypto::Aes128KeyByteArray));

    CHIP_ERROR err = AddressResolve::Resolver::Instance().LookupNode(request, mAddressLookupHandle);

    if (err == CHIP_NO_ERROR)
    {
        ICDNotifier::GetInstance().BroadcastActiveRequestNotification(ICDListener::KeepActiveFlag::kCheckInInProgress);
    }

    return err;
}

} // namespace app
} // namespace chip
