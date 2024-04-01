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

#include <app/SpecificationDefinedRevisions.h>
#include <lib/core/CHIPConfig.h>
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
    secureSession->Activate(GetLocalScopedNodeId(), GetPeer(), GetPeerCATs(), peerSessionId, GetRemoteSessionParameters());

    ChipLogDetail(Inet, "New secure session activated for device " ChipLogFormatScopedNodeId ", LSID:%d PSID:%d!",
                  ChipLogValueScopedNodeId(GetPeer()), secureSession->GetLocalSessionId(), peerSessionId);

    return CHIP_NO_ERROR;
}

void PairingSession::Finish()
{
    Transport::PeerAddress address = mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->GetPeerAddress();

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
    if (mExchangeCtxt.HasValue())
    {
        // Make sure the exchange doesn't try to notify us when it closes,
        // since we might be dead by then.
        mExchangeCtxt.Value()->SetDelegate(nullptr);

        // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        mExchangeCtxt.ClearValue();
    }
}

CHIP_ERROR PairingSession::EncodeSessionParameters(TLV::Tag tag, const ReliableMessageProtocolConfig & mrpLocalConfig,
                                                   TLV::TLVWriter & tlvWriter)
{
    TLV::TLVType mrpParamsContainer;
    ReturnErrorOnFailure(tlvWriter.StartContainer(tag, TLV::kTLVType_Structure, mrpParamsContainer));
    ReturnErrorOnFailure(
        tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kSessionIdleInterval), mrpLocalConfig.mIdleRetransTimeout.count()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kSessionActiveInterval),
                                       mrpLocalConfig.mActiveRetransTimeout.count()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kSessionActiveThreshold),
                                       mrpLocalConfig.mActiveThresholdTime.count()));

    uint16_t dataModel = Revision::kDataModelRevision;
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kDataModelRevision), dataModel));

    uint16_t interactionModel = Revision::kInteractionModelRevision;
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kInteractionModelRevision), interactionModel));

    uint32_t specVersion = Revision::kSpecificationVersion;
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kSpecificationVersion), specVersion));

    uint16_t maxPathsPerInvoke = CHIP_CONFIG_MAX_PATHS_PER_INVOKE;
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(SessionParameters::Tag::kMaxPathsPerInvoke), maxPathsPerInvoke));
    return tlvWriter.EndContainer(mrpParamsContainer);
}

CHIP_ERROR PairingSession::DecodeMRPParametersIfPresent(TLV::Tag expectedTag, TLV::ContiguousBufferTLVReader & tlvReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // The MRP parameters are optional.
    if (tlvReader.GetTag() != expectedTag)
    {
        return CHIP_NO_ERROR;
    }

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(tlvReader.Next());

    ChipLogDetail(SecureChannel, "Found MRP parameters in the message");

    // All TLV elements in the structure are optional. If the first element is present, process it and move
    // the TLV reader to the next element.
    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kSessionIdleInterval)
    {
        uint32_t idleRetransTimeout;
        ReturnErrorOnFailure(tlvReader.Get(idleRetransTimeout));
        mRemoteSessionParams.SetMRPIdleRetransTimeout(System::Clock::Milliseconds32(idleRetransTimeout));

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kSessionActiveInterval)
    {
        uint32_t activeRetransTimeout;
        ReturnErrorOnFailure(tlvReader.Get(activeRetransTimeout));
        mRemoteSessionParams.SetMRPActiveRetransTimeout(System::Clock::Milliseconds32(activeRetransTimeout));

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kSessionActiveThreshold)
    {
        uint16_t activeThresholdTime;
        ReturnErrorOnFailure(tlvReader.Get(activeThresholdTime));
        mRemoteSessionParams.SetMRPActiveThresholdTime(System::Clock::Milliseconds16(activeThresholdTime));

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kDataModelRevision)
    {
        uint16_t dataModelRevision;
        ReturnErrorOnFailure(tlvReader.Get(dataModelRevision));
        mRemoteSessionParams.SetDataModelRevision(dataModelRevision);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kInteractionModelRevision)
    {
        uint16_t interactionModelRevision;
        ReturnErrorOnFailure(tlvReader.Get(interactionModelRevision));
        mRemoteSessionParams.SetInteractionModelRevision(interactionModelRevision);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kSpecificationVersion)
    {
        uint32_t specificationVersion;
        ReturnErrorOnFailure(tlvReader.Get(specificationVersion));
        mRemoteSessionParams.SetSpecificationVersion(specificationVersion);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    if (TLV::TagNumFromTag(tlvReader.GetTag()) == SessionParameters::Tag::kMaxPathsPerInvoke)
    {
        uint16_t maxPathsPerInvoke;
        ReturnErrorOnFailure(tlvReader.Get(maxPathsPerInvoke));
        mRemoteSessionParams.SetMaxPathsPerInvoke(maxPathsPerInvoke);

        // The next element is optional. If it's not present, return CHIP_NO_ERROR.
        SuccessOrExit(err = tlvReader.Next());
    }

    // Future proofing - Don't error out if there are other tags
exit:
    if (err == CHIP_END_OF_TLV)
    {
        return tlvReader.ExitContainer(containerType);
    }
    return err;
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
    // Clear acts like the destructor of PairingSession. If it is called during
    // the middle of pairing, that means we should terminate the exchange. For the
    // normal path, the exchange should already be discarded before calling Clear.
    if (mExchangeCtxt.HasValue())
    {
        // The only time we reach this is when we are getting destroyed in the
        // middle of our handshake. In that case, there is no point in trying to
        // do MRP resends of the last message we sent. So, abort the exchange
        // instead of just closing it.
        mExchangeCtxt.Value()->Abort();
        mExchangeCtxt.ClearValue();
    }
    mSecureSessionHolder.Release();
    mPeerSessionId.ClearValue();
    mSessionManager = nullptr;
}

void PairingSession::NotifySessionEstablishmentError(CHIP_ERROR error, SessionEstablishmentStage stage)
{
    if (mDelegate == nullptr)
    {
        // Already notified success or error.
        return;
    }

    auto * delegate = mDelegate;
    mDelegate       = nullptr;
    delegate->OnSessionEstablishmentError(error, stage);
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
