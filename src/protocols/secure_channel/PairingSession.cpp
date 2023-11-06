/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <protocols/secure_channel/PairingSession.h>

#include <lib/core/TLVTypes.h>
#include <lib/support/SafeInt.h>

namespace chip {

CHIP_ERROR PairingSession::AllocateSecureSession(SessionManager & sessionManager, const ScopedNodeId & sessionEvictionHint)
{
    auto handle = sessionManager.AllocateSession(GetSecureSessionType(), sessionEvictionHint);
    VerifyOrReturnError(handle.HasValue(), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(mSecureSessionHolder.GrabPairingSession(handle.Value()), CHIP_ERROR_INTERNAL);
    mSessionManager = &sessionManager;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingSession::ActivateSecureSession(const Transport::PeerAddress & peerAddress)
{
    // Prepare SecureSession fields, including key derivation, first, before activation
    Transport::SecureSession * secureSession = mSecureSessionHolder->AsSecureSession();
    ReturnErrorOnFailure(DeriveSecureSession(secureSession->GetCryptoContext()));

    uint16_t peerSessionId = GetPeerSessionId();
    secureSession->SetPeerAddress(peerAddress);
    secureSession->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(Transport::PeerMessageCounter::kInitialSyncValue);

    // Call Activate last, otherwise errors on anything after would lead to
    // a partially valid session.
    secureSession->Activate(GetLocalScopedNodeId(), GetPeer(), GetPeerCATs(), peerSessionId, mRemoteMRPConfig);

    ChipLogDetail(Inet, "New secure session activated for device " ChipLogFormatScopedNodeId ", LSID:%d PSID:%d!",
                  ChipLogValueScopedNodeId(GetPeer()), secureSession->GetLocalSessionId(), peerSessionId);

    return CHIP_NO_ERROR;
}

void PairingSession::Finish()
{
    Transport::PeerAddress address = mExchangeCtxt->GetSessionHandle()->AsUnauthenticatedSession()->GetPeerAddress();

    // Discard the exchange so that Clear() doesn't try closing it. The exchange will handle that.
    DiscardExchange();

    CHIP_ERROR err = ActivateSecureSession(address);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrDie(mSecureSessionHolder);
        // Make sure to null out mDelegate so we don't send it any other
        // notifications.
        auto * delegate = mDelegate;
        mDelegate       = nullptr;
        delegate->OnSessionEstablished(mSecureSessionHolder.Get().Value());
    }
    else
    {
        NotifySessionEstablishmentError(err);
    }
}

void PairingSession::DiscardExchange()
{
    if (mExchangeCtxt != nullptr)
    {
        // Make sure the exchange doesn't try to notify us when it closes,
        // since we might be dead by then.
        mExchangeCtxt->SetDelegate(nullptr);
        // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        mExchangeCtxt = nullptr;
    }
}

CHIP_ERROR PairingSession::EncodeMRPParameters(TLV::Tag tag, const ReliableMessageProtocolConfig & mrpLocalConfig,
                                               TLV::TLVWriter & tlvWriter)
{
    TLV::TLVType mrpParamsContainer;
    ReturnErrorOnFailure(tlvWriter.StartContainer(tag, TLV::kTLVType_Structure, mrpParamsContainer));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), mrpLocalConfig.mIdleRetransTimeout.count()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), mrpLocalConfig.mActiveRetransTimeout.count()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), mrpLocalConfig.mActiveThresholdTime.count()));
    return tlvWriter.EndContainer(mrpParamsContainer);
}

CHIP_ERROR PairingSession::DecodeMRPParametersIfPresent(TLV::Tag expectedTag, TLV::ContiguousBufferTLVReader & tlvReader)
{
    // The MRP parameters are optional.
    if (tlvReader.GetTag() != expectedTag)
    {
        return CHIP_NO_ERROR;
    }

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    uint32_t tlvElementValue = 0;

    ReturnErrorOnFailure(tlvReader.Next());

    ChipLogDetail(SecureChannel, "Found MRP parameters in the message");

    // All TLV elements in the structure are optional. If the first element is present, process it and move
    // the TLV reader to the next element.
    if (TLV::TagNumFromTag(tlvReader.GetTag()) == 1)
    {
        ReturnErrorOnFailure(tlvReader.Get(tlvElementValue));
        mRemoteMRPConfig.mIdleRetransTimeout = System::Clock::Milliseconds32(tlvElementValue);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        CHIP_ERROR err = tlvReader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            return tlvReader.ExitContainer(containerType);
        }
        ReturnErrorOnFailure(err);
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == 2)
    {
        ReturnErrorOnFailure(tlvReader.Get(tlvElementValue));
        mRemoteMRPConfig.mActiveRetransTimeout = System::Clock::Milliseconds32(tlvElementValue);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        CHIP_ERROR err = tlvReader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            return tlvReader.ExitContainer(containerType);
        }
        ReturnErrorOnFailure(err);
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == 3)
    {
        ReturnErrorOnFailure(tlvReader.Get(tlvElementValue));
        mRemoteMRPConfig.mActiveThresholdTime = System::Clock::Milliseconds16(tlvElementValue);
    }
    // Future proofing - Don't error out if there are other tags

    return tlvReader.ExitContainer(containerType);
}

bool PairingSession::IsSessionEstablishmentInProgress()
{
    if (!mSecureSessionHolder)
    {
        return false;
    }

    Transport::SecureSession * secureSession = mSecureSessionHolder->AsSecureSession();
    return secureSession->IsEstablishing();
}

void PairingSession::Clear()
{
    // Clear acts like the destructor if PairingSession, if it is call during
    // middle of a pairing, means we should terminate the exchange. For normal
    // path, the exchange should already be discarded before calling Clear.
    if (mExchangeCtxt != nullptr)
    {
        // The only time we reach this is if we are getting destroyed in the
        // middle of our handshake.  In that case, there is no point trying to
        // do MRP resends of the last message we sent, so abort the exchange
        // instead of just closing it.
        mExchangeCtxt->Abort();
        mExchangeCtxt = nullptr;
    }

    mSecureSessionHolder.Release();
    mPeerSessionId.ClearValue();
    mSessionManager = nullptr;
}

void PairingSession::NotifySessionEstablishmentError(CHIP_ERROR error)
{
    if (mDelegate == nullptr)
    {
        // Already notified success or error.
        return;
    }

    auto * delegate = mDelegate;
    mDelegate       = nullptr;
    delegate->OnSessionEstablishmentError(error);
}

void PairingSession::OnSessionReleased()
{
    if (mRole == CryptoContext::SessionRole::kInitiator)
    {
        NotifySessionEstablishmentError(CHIP_ERROR_CONNECTION_ABORTED);
        return;
    }

    // Send the error notification async, because our delegate is likely to want
    // to create a new session to listen for new connection attempts, and doing
    // that under an OnSessionReleased notification is not safe.
    if (!mSessionManager)
    {
        return;
    }

    mSessionManager->SystemLayer()->ScheduleWork(
        [](auto * systemLayer, auto * appState) -> void {
            ChipLogError(Inet, "ASYNC CASE Session establishment failed");
            auto * _this = static_cast<PairingSession *>(appState);
            _this->NotifySessionEstablishmentError(CHIP_ERROR_CONNECTION_ABORTED);
        },
        this);
}

} // namespace chip
