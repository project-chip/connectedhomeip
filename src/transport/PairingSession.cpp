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

#include <transport/PairingSession.h>

#include <lib/core/CHIPTLVTypes.h>
#include <lib/support/SafeInt.h>

namespace chip {

CHIP_ERROR PairingSession::AllocateSecureSession(SessionManager & sessionManager)
{
    auto handle = sessionManager.AllocateSession();
    VerifyOrReturnError(handle.HasValue(), CHIP_ERROR_NO_MEMORY);
    mSecureSessionHolder.Grab(handle.Value());
    mSessionManager = &sessionManager;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingSession::ActivateSecureSession(const Transport::PeerAddress & peerAddress)
{
    // TODO(17568): Replace with proper expiry logic. This current method makes sure there
    // are not multiple sessions established, until eventual exhaustion of the resources
    // for CASE sessions. Current method is quick fix for #17698, cannot remain.
    mSessionManager->ExpireAllPairingsForPeerExceptPending(GetPeer());

    // Prepare SecureSession fields, including key derivation, first, before activation
    Transport::SecureSession * secureSession = mSecureSessionHolder->AsSecureSession();
    ReturnErrorOnFailure(DeriveSecureSession(secureSession->GetCryptoContext()));
    uint16_t peerSessionId = GetPeerSessionId();
    secureSession->SetPeerAddress(peerAddress);
    secureSession->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(LocalSessionMessageCounter::kInitialSyncValue);

    // Call Activate last, otherwise errors on anything after would lead to
    // a partially valid session.
    secureSession->Activate(GetSecureSessionType(), GetLocalScopedNodeId(), GetPeer(), GetPeerCATs(), peerSessionId,
                            mRemoteMRPConfig);

    ChipLogDetail(Inet, "New secure session created for device " ChipLogFormatScopedNodeId ", LSID:%d PSID:%d!",
                  ChipLogValueScopedNodeId(GetPeer()), secureSession->GetLocalSessionId(), peerSessionId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingSession::EncodeMRPParameters(TLV::Tag tag, const ReliableMessageProtocolConfig & mrpConfig,
                                               TLV::TLVWriter & tlvWriter)
{
    VerifyOrReturnError(CanCastTo<uint16_t>(mrpConfig.mIdleRetransTimeout.count()), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(mrpConfig.mActiveRetransTimeout.count()), CHIP_ERROR_INVALID_ARGUMENT);

    TLV::TLVType mrpParamsContainer;
    ReturnErrorOnFailure(tlvWriter.StartContainer(tag, TLV::kTLVType_Structure, mrpParamsContainer));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), static_cast<uint16_t>(mrpConfig.mIdleRetransTimeout.count())));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), static_cast<uint16_t>(mrpConfig.mActiveRetransTimeout.count())));
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

    uint16_t tlvElementValue = 0;

    ReturnErrorOnFailure(tlvReader.Next());

    ChipLogDetail(SecureChannel, "Found MRP parameters in the message");

    // Both TLV elements in the structure are optional. If the first element is present, process it and move
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

    VerifyOrReturnError(TLV::TagNumFromTag(tlvReader.GetTag()) == 2, CHIP_ERROR_INVALID_TLV_TAG);
    ReturnErrorOnFailure(tlvReader.Get(tlvElementValue));
    mRemoteMRPConfig.mActiveRetransTimeout = System::Clock::Milliseconds32(tlvElementValue);

    return tlvReader.ExitContainer(containerType);
}

void PairingSession::Clear()
{
    if (mSessionManager != nullptr)
    {
        if (mSecureSessionHolder && !mSecureSessionHolder->AsSecureSession()->IsActiveSession())
        {
            // Make sure to clean up our pending session, since we're the only
            // ones who have access to it do do so.
            mSessionManager->ExpirePairing(mSecureSessionHolder.Get());
        }
    }

    mPeerSessionId.ClearValue();
    // If we called ExpirePairing above, the holder has already released the
    // session (due to it being destroyed).  If not, we need to release it.
    // Release is idempotent, so it's OK to just call it here.
    mSecureSessionHolder.Release();
    mSessionManager = nullptr;
}

} // namespace chip
