/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *      This file implements the the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 *
 */
#include <protocols/secure_channel/CASESession.h>

#include <inttypes.h>
#include <string.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TypeTraits.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/CASEDestinationId.h>
#include <protocols/secure_channel/PairingSession.h>
#include <protocols/secure_channel/SessionResumptionStorage.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <trace/trace.h>
#include <transport/SessionManager.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

namespace {

enum
{
    kTag_TBEData_SenderNOC    = 1,
    kTag_TBEData_SenderICAC   = 2,
    kTag_TBEData_Signature    = 3,
    kTag_TBEData_ResumptionID = 4,
};

enum
{
    kTag_Sigma1_InitiatorRandom    = 1,
    kTag_Sigma1_InitiatorSessionId = 2,
    kTag_Sigma1_DestinationId      = 3,
    kTag_Sigma1_InitiatorEphPubKey = 4,
    kTag_Sigma1_InitiatorMRPParams = 5,
    kTag_Sigma1_ResumptionID       = 6,
    kTag_Sigma1_InitiatorResumeMIC = 7,
};

enum
{
    kTag_Sigma2_ResponderRandom    = 1,
    kTag_Sigma2_ResponderSessionId = 2,
    kTag_Sigma2_ResponderEphPubKey = 3,
    kTag_Sigma2_Encrypted2         = 4,
    kTag_Sigma2_ResponderMRPParams = 5,
};

enum
{
    kTag_Sigma3_Encrypted3 = 1,
};

} // namespace

namespace chip {

using namespace Crypto;
using namespace Credentials;
using namespace Messaging;
using namespace Encoding;
using namespace Protocols::SecureChannel;

constexpr uint8_t kKDFSR2Info[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32 };
constexpr uint8_t kKDFSR3Info[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33 };
constexpr size_t kKDFInfoLength = sizeof(kKDFSR2Info);

constexpr uint8_t kKDFS1RKeyInfo[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x31, 0x5f, 0x52, 0x65, 0x73, 0x75, 0x6d, 0x65 };
constexpr uint8_t kKDFS2RKeyInfo[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x5f, 0x52, 0x65, 0x73, 0x75, 0x6d, 0x65 };

constexpr uint8_t kResume1MIC_Nonce[] =
    /* "NCASE_SigmaR1" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x53, 0x31 };
constexpr uint8_t kResume2MIC_Nonce[] =
    /* "NCASE_SigmaR2" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x53, 0x32 };
constexpr uint8_t kTBEData2_Nonce[] =
    /* "NCASE_Sigma2N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x4e };
constexpr uint8_t kTBEData3_Nonce[] =
    /* "NCASE_Sigma3N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33, 0x4e };
constexpr size_t kTBEDataNonceLength = sizeof(kTBEData2_Nonce);
static_assert(sizeof(kTBEData2_Nonce) == sizeof(kTBEData3_Nonce), "TBEData2_Nonce and TBEData3_Nonce must be same size");

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

// Wait at most 30 seconds for the response from the peer.
// This timeout value assumes the underlying transport is reliable.
// The session establishment fails if the response is not received within timeout window.
static constexpr ExchangeContext::Timeout kSigma_Response_Timeout = System::Clock::Seconds16(30);

CASESession::~CASESession()
{
    // Let's clear out any security state stored in the object, before destroying it.
    Clear();
}

void CASESession::OnSessionReleased()
{
    Clear();
    // Do this last in case the delegate frees us.
    NotifySessionEstablishmentError(CHIP_ERROR_CONNECTION_ABORTED);
}

void CASESession::Clear()
{
    // This function zeroes out and resets the memory used by the object.
    // It's done so that no security related information will be leaked.
    mCommissioningHash.Clear();
    PairingSession::Clear();

    mState = State::kInitialized;
    Crypto::ClearSecretData(mIPK);

    if (mFabricsTable != nullptr)
    {
        mFabricsTable->RemoveFabricDelegate(this);

        mFabricsTable->ReleaseEphemeralKeypair(mEphemeralKey);
        mEphemeralKey = nullptr;
    }

    mLocalNodeId  = kUndefinedNodeId;
    mPeerNodeId   = kUndefinedNodeId;
    mFabricsTable = nullptr;
    mFabricIndex  = kUndefinedFabricIndex;
}

void CASESession::InvalidateIfPendingEstablishmentOnFabric(FabricIndex fabricIndex)
{
    if (mFabricIndex != fabricIndex)
    {
        return;
    }
    if (!IsSessionEstablishmentInProgress())
    {
        return;
    }
    AbortPendingEstablish(CHIP_ERROR_CANCELLED);
}

CHIP_ERROR CASESession::Init(SessionManager & sessionManager, Credentials::CertificateValidityPolicy * policy,
                             SessionEstablishmentDelegate * delegate, const ScopedNodeId & sessionEvictionHint)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mGroupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());

    mDelegate = delegate;
    ReturnErrorOnFailure(AllocateSecureSession(sessionManager, sessionEvictionHint));

    mValidContext.Reset();
    mValidContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    mValidContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    mValidContext.mValidityPolicy = policy;

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CASESession::PrepareForSessionEstablishment(SessionManager & sessionManager, FabricTable * fabricTable,
                                            SessionResumptionStorage * sessionResumptionStorage,
                                            Credentials::CertificateValidityPolicy * policy,
                                            SessionEstablishmentDelegate * delegate, const ScopedNodeId & previouslyEstablishedPeer,
                                            Optional<ReliableMessageProtocolConfig> mrpLocalConfig)
{
    // Below VerifyOrReturnError is not SuccessOrExit since we only want to goto `exit:` after
    // Init has been successfully called.
    VerifyOrReturnError(fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(Init(sessionManager, policy, delegate, previouslyEstablishedPeer));

    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err = fabricTable->AddFabricDelegate(this));

    mFabricsTable             = fabricTable;
    mRole                     = CryptoContext::SessionRole::kResponder;
    mSessionResumptionStorage = sessionResumptionStorage;
    mLocalMRPConfig           = mrpLocalConfig;

    ChipLogDetail(SecureChannel, "Allocated SecureSession (%p) - waiting for Sigma1 msg",
                  mSecureSessionHolder.Get().Value()->AsSecureSession());

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR CASESession::EstablishSession(SessionManager & sessionManager, FabricTable * fabricTable, ScopedNodeId peerScopedNodeId,
                                         ExchangeContext * exchangeCtxt, SessionResumptionStorage * sessionResumptionStorage,
                                         Credentials::CertificateValidityPolicy * policy, SessionEstablishmentDelegate * delegate,
                                         Optional<ReliableMessageProtocolConfig> mrpLocalConfig)
{
    MATTER_TRACE_EVENT_SCOPE("EstablishSession", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Return early on error here, as we have not initialized any state yet
    ReturnErrorCodeIf(exchangeCtxt == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(fabricTable == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Use FabricTable directly to avoid situation of dangling index from stale FabricInfo
    // until we factor-out any FabricInfo direct usage.
    ReturnErrorCodeIf(peerScopedNodeId.GetFabricIndex() == kUndefinedFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
    const auto * fabricInfo = fabricTable->FindFabricWithIndex(peerScopedNodeId.GetFabricIndex());
    ReturnErrorCodeIf(fabricInfo == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    err = Init(sessionManager, policy, delegate, peerScopedNodeId);

    mRole = CryptoContext::SessionRole::kInitiator;

    // We are setting the exchange context specifically before checking for error.
    // This is to make sure the exchange will get closed if Init() returned an error.
    mExchangeCtxt = exchangeCtxt;

    // From here onwards, let's go to exit on error, as some state might have already
    // been initialized
    SuccessOrExit(err);

    SuccessOrExit(err = fabricTable->AddFabricDelegate(this));

    mFabricsTable             = fabricTable;
    mFabricIndex              = fabricInfo->GetFabricIndex();
    mSessionResumptionStorage = sessionResumptionStorage;
    mLocalMRPConfig           = mrpLocalConfig;

    mExchangeCtxt->SetResponseTimeout(kSigma_Response_Timeout + mExchangeCtxt->GetSessionHandle()->GetAckTimeout());
    mPeerNodeId  = peerScopedNodeId.GetNodeId();
    mLocalNodeId = fabricInfo->GetNodeId();

    ChipLogProgress(SecureChannel, "Initiating session on local FabricIndex %u from 0x" ChipLogFormatX64 " -> 0x" ChipLogFormatX64,
                    static_cast<unsigned>(mFabricIndex), ChipLogValueX64(mLocalNodeId), ChipLogValueX64(mPeerNodeId));

    err = SendSigma1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

void CASESession::OnResponseTimeout(ExchangeContext * ec)
{
    VerifyOrReturn(ec != nullptr, ChipLogError(SecureChannel, "CASESession::OnResponseTimeout was called by null exchange"));
    VerifyOrReturn(mExchangeCtxt == ec, ChipLogError(SecureChannel, "CASESession::OnResponseTimeout exchange doesn't match"));
    ChipLogError(SecureChannel, "CASESession timed out while waiting for a response from the peer. Current state was %u",
                 to_underlying(mState));
    // Discard the exchange so that Clear() doesn't try aborting it.  The
    // exchange will handle that.
    DiscardExchange();
    AbortPendingEstablish(CHIP_ERROR_TIMEOUT);
}

void CASESession::AbortPendingEstablish(CHIP_ERROR err)
{
    Clear();
    // Do this last in case the delegate frees us.
    NotifySessionEstablishmentError(err);
}

CHIP_ERROR CASESession::DeriveSecureSession(CryptoContext & session) const
{
    switch (mState)
    {
    case State::kFinished: {
        std::array<uint8_t, sizeof(mIPK) + kSHA256_Hash_Length> msg_salt;

        {
            Encoding::LittleEndian::BufferWriter bbuf(msg_salt);
            bbuf.Put(mIPK, sizeof(mIPK));
            bbuf.Put(mMessageDigest, sizeof(mMessageDigest));

            VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
        }

        ReturnErrorOnFailure(session.InitFromSecret(ByteSpan(mSharedSecret, mSharedSecret.Length()), ByteSpan(msg_salt),
                                                    CryptoContext::SessionInfoType::kSessionEstablishment, mRole));

        return CHIP_NO_ERROR;
    }
    case State::kFinishedViaResume: {
        std::array<uint8_t, sizeof(mInitiatorRandom) + decltype(mResumeResumptionId)().size()> msg_salt;

        {
            Encoding::LittleEndian::BufferWriter bbuf(msg_salt);
            bbuf.Put(mInitiatorRandom, sizeof(mInitiatorRandom));
            bbuf.Put(mResumeResumptionId.data(), mResumeResumptionId.size());

            VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
        }

        ReturnErrorOnFailure(session.InitFromSecret(ByteSpan(mSharedSecret, mSharedSecret.Length()), ByteSpan(msg_salt),
                                                    CryptoContext::SessionInfoType::kSessionResumption, mRole));

        return CHIP_NO_ERROR;
    }
    default:
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

CHIP_ERROR CASESession::RecoverInitiatorIpk()
{
    Credentials::GroupDataProvider::KeySet ipkKeySet;

    CHIP_ERROR err = mGroupDataProvider->GetIpkKeySet(mFabricIndex, ipkKeySet);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to obtain IPK for initiating: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    if ((ipkKeySet.num_keys_used == 0) || (ipkKeySet.num_keys_used > Credentials::GroupDataProvider::KeySet::kEpochKeysMax))
    {
        ChipLogError(SecureChannel, "Found invalid IPK keyset for initiator.");
        return CHIP_ERROR_INTERNAL;
    }

    // For the generation of the Destination Identifier,
    // the originator SHALL use the operational group key with the second oldest
    // EpochStartTime, if one exists, otherwise it SHALL use the single operational
    // group key available. The EpochStartTime are already ordered
    size_t ipkIndex = (ipkKeySet.num_keys_used > 1) ? ((ipkKeySet.num_keys_used - 1) - 1) : 0;
    memcpy(&mIPK[0], ipkKeySet.epoch_keys[ipkIndex].key, sizeof(mIPK));

    // Leaving this logging code for debug, but this cannot be enabled at runtime
    // since it leaks private security material.
#if 0
    ChipLogProgress(SecureChannel, "RecoverInitiatorIpk: GroupDataProvider %p, Got IPK for FabricIndex %u", mGroupDataProvider,
                    static_cast<unsigned>(mFabricIndex));
    ChipLogByteSpan(SecureChannel, ByteSpan(mIPK));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma1()
{
    MATTER_TRACE_EVENT_SCOPE("SendSigma1", "CASESession");
    const size_t mrpParamsSize = mLocalMRPConfig.HasValue() ? TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t)) : 0;
    size_t data_len            = TLV::EstimateStructOverhead(kSigmaParamRandomNumberSize, // initiatorRandom
                                                  sizeof(uint16_t),            // initiatorSessionId,
                                                  kSHA256_Hash_Length,         // destinationId
                                                  kP256_PublicKey_Length,      // InitiatorEphPubKey,
                                                  mrpParamsSize,               // initiatorMRPParams
                                                  SessionResumptionStorage::kResumptionIdSize, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R1;
    TLV::TLVType outerContainerType                    = TLV::kTLVType_NotSpecified;
    uint8_t destinationIdentifier[kSHA256_Hash_Length] = { 0 };

    // Lookup fabric info.
    const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(mFabricIndex);
    ReturnErrorCodeIf(fabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Validate that we have a session ID allocated.
    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);

    // Generate an ephemeral keypair
    mEphemeralKey = mFabricsTable->AllocateEphemeralKeypairForCASE();
    VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(mEphemeralKey->Initialize());

    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(mInitiatorRandom, sizeof(mInitiatorRandom)));

    // Construct Sigma1 Msg
    msg_R1 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_ERROR_NO_MEMORY);

    tlvWriter.Init(std::move(msg_R1));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), ByteSpan(mInitiatorRandom)));
    // Retrieve Session Identifier
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), GetLocalSessionId().Value()));

    // Generate a Destination Identifier based on the node we are attempting to reach
    {
        // Obtain originator IPK matching the fabric where we are trying to open a session. mIPK
        // will be properly set thereafter.
        ReturnErrorOnFailure(RecoverInitiatorIpk());

        FabricId fabricId = fabricInfo->GetFabricId();
        Crypto::P256PublicKey rootPubKey;
        ReturnErrorOnFailure(mFabricsTable->FetchRootPubkey(mFabricIndex, rootPubKey));
        Credentials::P256PublicKeySpan rootPubKeySpan{ rootPubKey.ConstBytes() };

        MutableByteSpan destinationIdSpan(destinationIdentifier);
        ReturnErrorOnFailure(GenerateCaseDestinationId(ByteSpan(mIPK), ByteSpan(mInitiatorRandom), rootPubKeySpan, fabricId,
                                                       mPeerNodeId, destinationIdSpan));
    }
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(3), destinationIdentifier, sizeof(destinationIdentifier)));

    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(4), mEphemeralKey->Pubkey(), static_cast<uint32_t>(mEphemeralKey->Pubkey().Length())));

    if (mLocalMRPConfig.HasValue())
    {
        ChipLogDetail(SecureChannel, "Including MRP parameters");
        ReturnErrorOnFailure(EncodeMRPParameters(TLV::ContextTag(5), mLocalMRPConfig.Value(), tlvWriter));
    }

    // Try to find persistent session, and resume it.
    bool resuming = false;
    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err = mSessionResumptionStorage->FindByScopedNodeId(fabricInfo->GetScopedNodeIdForNode(mPeerNodeId),
                                                                       mResumeResumptionId, mSharedSecret, mPeerCATs);
        if (err == CHIP_NO_ERROR)
        {
            // Found valid resumption state, try to resume the session.
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(6), mResumeResumptionId));

            uint8_t initiatorResume1MIC[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];
            MutableByteSpan resumeMICSpan(initiatorResume1MIC);
            ReturnErrorOnFailure(GenerateSigmaResumeMIC(ByteSpan(mInitiatorRandom), ByteSpan(mResumeResumptionId),
                                                        ByteSpan(kKDFS1RKeyInfo), ByteSpan(kResume1MIC_Nonce), resumeMICSpan));

            ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(7), resumeMICSpan));
            resuming = true;
        }
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R1));

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msg_R1->Start(), msg_R1->DataLength() }));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma1, std::move(msg_R1),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    mState = resuming ? State::kSentSigma1Resume : State::kSentSigma1;

    ChipLogProgress(SecureChannel, "Sent Sigma1 msg");

    mDelegate->OnSessionEstablishmentStarted();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma1_and_SendSigma2(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_EVENT_SCOPE("HandleSigma1_and_SendSigma2", "CASESession");
    ReturnErrorOnFailure(HandleSigma1(std::move(msg)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FindLocalNodeFromDestinationId(const ByteSpan & destinationId, const ByteSpan & initiatorRandom)
{
    VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);

    bool found = false;
    for (const FabricInfo & fabricInfo : *mFabricsTable)
    {
        // Basic data for candidate fabric, used to compute candidate destination identifiers
        FabricId fabricId = fabricInfo.GetFabricId();
        NodeId nodeId     = fabricInfo.GetNodeId();
        Crypto::P256PublicKey rootPubKey;
        ReturnErrorOnFailure(mFabricsTable->FetchRootPubkey(fabricInfo.GetFabricIndex(), rootPubKey));
        Credentials::P256PublicKeySpan rootPubKeySpan{ rootPubKey.ConstBytes() };

        // Get IPK operational group key set for current candidate fabric
        GroupDataProvider::KeySet ipkKeySet;
        CHIP_ERROR err = mGroupDataProvider->GetIpkKeySet(fabricInfo.GetFabricIndex(), ipkKeySet);
        if ((err != CHIP_NO_ERROR) ||
            ((ipkKeySet.num_keys_used == 0) || (ipkKeySet.num_keys_used > Credentials::GroupDataProvider::KeySet::kEpochKeysMax)))
        {
            continue;
        }

        // Try every IPK candidate we have for a match
        for (size_t keyIdx = 0; keyIdx < ipkKeySet.num_keys_used; ++keyIdx)
        {
            uint8_t candidateDestinationId[kSHA256_Hash_Length];
            MutableByteSpan candidateDestinationIdSpan(candidateDestinationId);
            ByteSpan candidateIpkSpan(ipkKeySet.epoch_keys[keyIdx].key);

            err = GenerateCaseDestinationId(ByteSpan(candidateIpkSpan), ByteSpan(initiatorRandom), rootPubKeySpan, fabricId, nodeId,
                                            candidateDestinationIdSpan);
            if ((err == CHIP_NO_ERROR) && (candidateDestinationIdSpan.data_equal(destinationId)))
            {
                // Found a match, stop working, cache IPK, update local fabric context
                found = true;
                MutableByteSpan ipkSpan(mIPK);
                CopySpanToMutableSpan(candidateIpkSpan, ipkSpan);
                mFabricIndex = fabricInfo.GetFabricIndex();
                mLocalNodeId = nodeId;
                break;
            }
        }

        if (found)
        {
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR CASESession::TryResumeSession(SessionResumptionStorage::ConstResumptionIdView resumptionId, ByteSpan resume1MIC,
                                         ByteSpan initiatorRandom)
{
    VerifyOrReturnError(mSessionResumptionStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);

    SessionResumptionStorage::ConstResumptionIdView resumptionIdSpan(resumptionId);
    ScopedNodeId node;
    ReturnErrorOnFailure(mSessionResumptionStorage->FindByResumptionId(resumptionIdSpan, node, mSharedSecret, mPeerCATs));

    // Cross check resume1MIC with the shared secret
    ReturnErrorOnFailure(
        ValidateSigmaResumeMIC(resume1MIC, initiatorRandom, resumptionId, ByteSpan(kKDFS1RKeyInfo), ByteSpan(kResume1MIC_Nonce)));

    const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(node.GetFabricIndex());
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mFabricIndex = node.GetFabricIndex();
    mPeerNodeId  = node.GetNodeId();
    mLocalNodeId = fabricInfo->GetNodeId();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma1(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_EVENT_SCOPE("HandleSigma1", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;

    uint16_t initiatorSessionId;
    ByteSpan destinationIdentifier;
    ByteSpan initiatorRandom;

    ChipLogProgress(SecureChannel, "Received Sigma1 msg");

    bool sessionResumptionRequested = false;
    ByteSpan resumptionId;
    ByteSpan resume1MIC;
    ByteSpan initiatorPubKey;

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = ParseSigma1(tlvReader, initiatorRandom, initiatorSessionId, destinationIdentifier, initiatorPubKey,
                                    sessionResumptionRequested, resumptionId, resume1MIC));

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", initiatorSessionId);
    SetPeerSessionId(initiatorSessionId);

    VerifyOrExit(mFabricsTable != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    if (sessionResumptionRequested && resumptionId.size() == SessionResumptionStorage::kResumptionIdSize &&
        CHIP_NO_ERROR ==
            TryResumeSession(SessionResumptionStorage::ConstResumptionIdView(resumptionId.data()), resume1MIC, initiatorRandom))
    {
        std::copy(initiatorRandom.begin(), initiatorRandom.end(), mInitiatorRandom);
        std::copy(resumptionId.begin(), resumptionId.end(), mResumeResumptionId.begin());

        // Send Sigma2Resume message to the initiator
        SuccessOrExit(err = SendSigma2Resume());

        mDelegate->OnSessionEstablishmentStarted();

        // Early returning here, since we have sent Sigma2Resume, and no further processing is needed for the Sigma1 message
        return CHIP_NO_ERROR;
    }

    // Attempt to match the initiator's desired destination based on local fabric table.
    err = FindLocalNodeFromDestinationId(destinationIdentifier, initiatorRandom);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(SecureChannel, "CASE matched destination ID: fabricIndex %u, NodeID 0x" ChipLogFormatX64,
                        static_cast<unsigned>(mFabricIndex), ChipLogValueX64(mLocalNodeId));

        // Side-effect of FindLocalNodeFromDestinationId success was that mFabricIndex/mLocalNodeId are now
        // set to the local fabric and associated NodeId that was targeted by the initiator.
    }
    else
    {
        ChipLogError(SecureChannel, "CASE failed to match destination ID with local fabrics");
        ChipLogByteSpan(SecureChannel, destinationIdentifier);
    }

    // ParseSigma1 ensures that:
    // mRemotePubKey.Length() == initiatorPubKey.size() == kP256_PublicKey_Length.
    memcpy(mRemotePubKey.Bytes(), initiatorPubKey.data(), mRemotePubKey.Length());

    SuccessOrExit(err = SendSigma2());

    mDelegate->OnSessionEstablishmentStarted();

exit:

    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeNoSharedRoot);
        mState = State::kInitialized;
    }
    else if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        mState = State::kInitialized;
    }
    return err;
}

CHIP_ERROR CASESession::SendSigma2Resume()
{
    MATTER_TRACE_EVENT_SCOPE("SendSigma2Resume", "CASESession");
    const size_t mrpParamsSize = mLocalMRPConfig.HasValue() ? TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t)) : 0;
    size_t max_sigma2_resume_data_len = TLV::EstimateStructOverhead(
        SessionResumptionStorage::kResumptionIdSize, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, sizeof(uint16_t), mrpParamsSize);

    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R2_resume;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    // Validate that we have a session ID allocated.
    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);

    msg_R2_resume = System::PacketBufferHandle::New(max_sigma2_resume_data_len);
    VerifyOrReturnError(!msg_R2_resume.IsNull(), CHIP_ERROR_NO_MEMORY);

    tlvWriter.Init(std::move(msg_R2_resume));

    // Generate a new resumption ID
    ReturnErrorOnFailure(DRBG_get_bytes(mNewResumptionId.data(), mNewResumptionId.size()));

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), mNewResumptionId));

    uint8_t sigma2ResumeMIC[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];
    MutableByteSpan resumeMICSpan(sigma2ResumeMIC);
    ReturnErrorOnFailure(GenerateSigmaResumeMIC(ByteSpan(mInitiatorRandom), mNewResumptionId, ByteSpan(kKDFS2RKeyInfo),
                                                ByteSpan(kResume2MIC_Nonce), resumeMICSpan));

    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), resumeMICSpan));

    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), GetLocalSessionId().Value()));

    if (mLocalMRPConfig.HasValue())
    {
        ChipLogDetail(SecureChannel, "Including MRP parameters");
        ReturnErrorOnFailure(EncodeMRPParameters(TLV::ContextTag(4), mLocalMRPConfig.Value(), tlvWriter));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R2_resume));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma2Resume, std::move(msg_R2_resume),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    mState = State::kSentSigma2Resume;

    ChipLogDetail(SecureChannel, "Sent Sigma2Resume msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma2()
{
    MATTER_TRACE_EVENT_SCOPE("SendSigma2", "CASESession");

    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icacBuf;
    VerifyOrReturnError(icacBuf.Alloc(kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

    chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
    VerifyOrReturnError(nocBuf.Alloc(kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan icaCert{ icacBuf.Get(), kMaxCHIPCertLength };
    ReturnErrorOnFailure(mFabricsTable->FetchICACert(mFabricIndex, icaCert));

    MutableByteSpan nocCert{ nocBuf.Get(), kMaxCHIPCertLength };
    ReturnErrorOnFailure(mFabricsTable->FetchNOCCert(mFabricIndex, nocCert));

    // Fill in the random value
    uint8_t msg_rand[kSigmaParamRandomNumberSize];
    ReturnErrorOnFailure(DRBG_get_bytes(&msg_rand[0], sizeof(msg_rand)));

    // Generate an ephemeral keypair
    mEphemeralKey = mFabricsTable->AllocateEphemeralKeypairForCASE();
    VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(mEphemeralKey->Initialize());

    // Generate a Shared Secret
    ReturnErrorOnFailure(mEphemeralKey->ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    uint8_t msg_salt[kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length];

    MutableByteSpan saltSpan(msg_salt);
    ReturnErrorOnFailure(ConstructSaltSigma2(ByteSpan(msg_rand), mEphemeralKey->Pubkey(), ByteSpan(mIPK), saltSpan));

    HKDF_sha_crypto mHKDF;
    uint8_t sr2k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR2Info,
                                           kKDFInfoLength, sr2k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES));

    // Construct Sigma2 TBS Data
    size_t msg_r2_signed_len =
        TLV::EstimateStructOverhead(kMaxCHIPCertLength, kMaxCHIPCertLength, kP256_PublicKey_Length, kP256_PublicKey_Length);

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    VerifyOrReturnError(msg_R2_Signed.Alloc(msg_r2_signed_len), CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(ConstructTBSData(nocCert, icaCert, ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()),
                                          ByteSpan(mRemotePubKey, mRemotePubKey.Length()), msg_R2_Signed.Get(), msg_r2_signed_len));

    // Generate a Signature
    P256ECDSASignature tbsData2Signature;
    ReturnErrorOnFailure(
        mFabricsTable->SignWithOpKeypair(mFabricIndex, ByteSpan{ msg_R2_Signed.Get(), msg_r2_signed_len }, tbsData2Signature));
    msg_R2_Signed.Free();

    // Construct Sigma2 TBE Data
    size_t msg_r2_signed_enc_len = TLV::EstimateStructOverhead(nocCert.size(), icaCert.size(), tbsData2Signature.Length(),
                                                               SessionResumptionStorage::kResumptionIdSize);

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    VerifyOrReturnError(msg_R2_Encrypted.Alloc(msg_r2_signed_enc_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES), CHIP_ERROR_NO_MEMORY);

    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderNOC), nocCert));
    if (!icaCert.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderICAC), icaCert));
    }

    // We are now done with ICAC and NOC certs so we can release the memory.
    {
        icacBuf.Free();
        icaCert = MutableByteSpan{};

        nocBuf.Free();
        nocCert = MutableByteSpan{};
    }

    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(kTag_TBEData_Signature), tbsData2Signature,
                                            static_cast<uint32_t>(tbsData2Signature.Length())));

    // Generate a new resumption ID
    ReturnErrorOnFailure(DRBG_get_bytes(mNewResumptionId.data(), mNewResumptionId.size()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBEData_ResumptionID), mNewResumptionId));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    msg_r2_signed_enc_len = static_cast<size_t>(tlvWriter.GetLengthWritten());

    // Generate the encrypted data blob
    ReturnErrorOnFailure(AES_CCM_encrypt(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len, nullptr, 0, sr2k,
                                         CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, kTBEData2_Nonce, kTBEDataNonceLength,
                                         msg_R2_Encrypted.Get(), msg_R2_Encrypted.Get() + msg_r2_signed_enc_len,
                                         CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    // Construct Sigma2 Msg
    const size_t mrpParamsSize = mLocalMRPConfig.HasValue() ? TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t)) : 0;
    size_t data_len            = TLV::EstimateStructOverhead(kSigmaParamRandomNumberSize, sizeof(uint16_t), kP256_PublicKey_Length,
                                                  msg_r2_signed_enc_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, mrpParamsSize);

    System::PacketBufferHandle msg_R2 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R2.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriterMsg2;
    outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriterMsg2.Init(std::move(msg_R2));
    ReturnErrorOnFailure(tlvWriterMsg2.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(TLV::ContextTag(1), &msg_rand[0], sizeof(msg_rand)));
    ReturnErrorOnFailure(tlvWriterMsg2.Put(TLV::ContextTag(2), GetLocalSessionId().Value()));
    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(TLV::ContextTag(3), mEphemeralKey->Pubkey(),
                                                static_cast<uint32_t>(mEphemeralKey->Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(TLV::ContextTag(4), msg_R2_Encrypted.Get(),
                                                static_cast<uint32_t>(msg_r2_signed_enc_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES)));
    if (mLocalMRPConfig.HasValue())
    {
        ChipLogDetail(SecureChannel, "Including MRP parameters");
        ReturnErrorOnFailure(EncodeMRPParameters(TLV::ContextTag(5), mLocalMRPConfig.Value(), tlvWriterMsg2));
    }
    ReturnErrorOnFailure(tlvWriterMsg2.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriterMsg2.Finalize(&msg_R2));

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msg_R2->Start(), msg_R2->DataLength() }));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma2, std::move(msg_R2),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    mState = State::kSentSigma2;

    ChipLogProgress(SecureChannel, "Sent Sigma2 msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma2Resume(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_EVENT_SCOPE("HandleSigma2Resume", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t responderSessionId;

    uint32_t decodeTagIdSeq = 0;

    ChipLogDetail(SecureChannel, "Received Sigma2Resume msg");

    uint8_t sigma2ResumeMIC[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SessionResumptionStorage::ResumptionIdStorage resumptionId;
    VerifyOrExit(tlvReader.GetLength() == resumptionId.size(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(resumptionId.data(), resumptionId.size()));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tlvReader.GetLength() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(sigma2ResumeMIC, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    SuccessOrExit(err = ValidateSigmaResumeMIC(ByteSpan(sigma2ResumeMIC), ByteSpan(mInitiatorRandom), resumptionId,
                                               ByteSpan(kKDFS2RKeyInfo), ByteSpan(kResume2MIC_Nonce)));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    if (tlvReader.Next() != CHIP_END_OF_TLV)
    {
        SuccessOrExit(err = DecodeMRPParametersIfPresent(TLV::ContextTag(4), tlvReader));
        mExchangeCtxt->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteMRPConfig(mRemoteMRPConfig);
    }

    ChipLogDetail(SecureChannel, "Peer assigned session session ID %d", responderSessionId);
    SetPeerSessionId(responderSessionId);

    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err2 = mSessionResumptionStorage->Save(GetPeer(), resumptionId, mSharedSecret, mPeerCATs);
        if (err2 != CHIP_NO_ERROR)
            ChipLogError(SecureChannel, "Unable to save session resumption state: %" CHIP_ERROR_FORMAT, err2.Format());
    }

    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    mState = State::kFinishedViaResume;
    Finish();

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigma2_and_SendSigma3(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_EVENT_SCOPE("HandleSigma2_and_SendSigma3", "CASESession");
    ReturnErrorOnFailure(HandleSigma2(std::move(msg)));
    ReturnErrorOnFailure(SendSigma3());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma2(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_EVENT_SCOPE("HandleSigma2", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf = msg->Start();
    size_t buflen       = msg->DataLength();

    uint8_t msg_salt[kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length];

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    size_t msg_r2_encrypted_len          = 0;
    size_t msg_r2_encrypted_len_with_tag = 0;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    size_t msg_r2_signed_len;
    size_t max_msg_r2_signed_enc_len;
    constexpr size_t kCaseOverheadForFutureTbeData = 128;

    uint8_t sr2k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    P256ECDSASignature tbsData2Signature;

    NodeId responderNodeId;
    P256PublicKey responderPublicKey;

    uint8_t responderRandom[kSigmaParamRandomNumberSize];
    ByteSpan responderNOC;
    ByteSpan responderICAC;

    uint16_t responderSessionId;

    VerifyOrExit(mEphemeralKey != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    ChipLogProgress(SecureChannel, "Received Sigma2 msg");

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Retrieve Responder's Random value
    SuccessOrExit(err = tlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_Sigma2_ResponderRandom)));
    SuccessOrExit(err = tlvReader.GetBytes(responderRandom, sizeof(responderRandom)));

    // Assign Session ID
    SuccessOrExit(err = tlvReader.Next(TLV::kTLVType_UnsignedInteger, TLV::ContextTag(kTag_Sigma2_ResponderSessionId)));
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session session ID %d", responderSessionId);
    SetPeerSessionId(responderSessionId);

    // Retrieve Responder's Ephemeral Pubkey
    SuccessOrExit(err = tlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_Sigma2_ResponderEphPubKey)));
    SuccessOrExit(err = tlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));

    // Generate a Shared Secret
    SuccessOrExit(err = mEphemeralKey->ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    // Generate the S2K key
    {
        MutableByteSpan saltSpan(msg_salt);
        err = ConstructSaltSigma2(ByteSpan(responderRandom), mRemotePubKey, ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);

        HKDF_sha_crypto mHKDF;
        err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR2Info,
                                kKDFInfoLength, sr2k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        SuccessOrExit(err);
    }

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, buflen }));

    // Generate decrypted data
    SuccessOrExit(err = tlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_Sigma2_Encrypted2)));

    max_msg_r2_signed_enc_len =
        TLV::EstimateStructOverhead(Credentials::kMaxCHIPCertLength, Credentials::kMaxCHIPCertLength, tbsData2Signature.Length(),
                                    SessionResumptionStorage::kResumptionIdSize, kCaseOverheadForFutureTbeData);
    msg_r2_encrypted_len_with_tag = tlvReader.GetLength();

    // Validate we did not receive a buffer larger than legal
    VerifyOrExit(msg_r2_encrypted_len_with_tag <= max_msg_r2_signed_enc_len, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrExit(msg_r2_encrypted_len_with_tag > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrExit(msg_R2_Encrypted.Alloc(msg_r2_encrypted_len_with_tag), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = tlvReader.GetBytes(msg_R2_Encrypted.Get(), static_cast<uint32_t>(msg_r2_encrypted_len_with_tag)));
    msg_r2_encrypted_len = msg_r2_encrypted_len_with_tag - CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

    SuccessOrExit(err = AES_CCM_decrypt(msg_R2_Encrypted.Get(), msg_r2_encrypted_len, nullptr, 0,
                                        msg_R2_Encrypted.Get() + msg_r2_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, sr2k,
                                        CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, kTBEData2_Nonce, kTBEDataNonceLength,
                                        msg_R2_Encrypted.Get()));

    decryptedDataTlvReader.Init(msg_R2_Encrypted.Get(), msg_r2_encrypted_len);
    containerType = TLV::kTLVType_Structure;
    SuccessOrExit(err = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = decryptedDataTlvReader.EnterContainer(containerType));

    SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_SenderNOC)));
    SuccessOrExit(err = decryptedDataTlvReader.Get(responderNOC));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    if (TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_SenderICAC)
    {
        VerifyOrExit(decryptedDataTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
        SuccessOrExit(err = decryptedDataTlvReader.Get(responderICAC));
        SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_Signature)));
    }

    // Validate responder identity located in msg_r2_encrypted
    // Constructing responder identity
    SuccessOrExit(err = ValidatePeerIdentity(responderNOC, responderICAC, responderNodeId, responderPublicKey));

    // Verify that responderNodeId (from responderNOC) matches one that was included
    // in the computation of the Destination Identifier when generating Sigma1.
    VerifyOrReturnError(mPeerNodeId == responderNodeId, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Construct msg_R2_Signed and validate the signature in msg_r2_encrypted
    msg_r2_signed_len = TLV::EstimateStructOverhead(sizeof(uint16_t), responderNOC.size(), responderICAC.size(),
                                                    kP256_PublicKey_Length, kP256_PublicKey_Length);

    VerifyOrExit(msg_R2_Signed.Alloc(msg_r2_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBSData(responderNOC, responderICAC, ByteSpan(mRemotePubKey, mRemotePubKey.Length()),
                                         ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()), msg_R2_Signed.Get(),
                                         msg_r2_signed_len));

    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_Signature, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tbsData2Signature.Capacity() >= decryptedDataTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    tbsData2Signature.SetLength(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(tbsData2Signature, tbsData2Signature.Length()));

    // Validate signature
    SuccessOrExit(err = responderPublicKey.ECDSA_validate_msg_signature(msg_R2_Signed.Get(), msg_r2_signed_len, tbsData2Signature));

    // Retrieve session resumption ID
    SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_ResumptionID)));
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(mNewResumptionId.data(), mNewResumptionId.size()));

    // Retrieve peer CASE Authenticated Tags (CATs) from peer's NOC.
    SuccessOrExit(err = ExtractCATsFromOpCert(responderNOC, mPeerCATs));

    // Retrieve responderMRPParams if present
    if (tlvReader.Next() != CHIP_END_OF_TLV)
    {
        SuccessOrExit(err = DecodeMRPParametersIfPresent(TLV::ContextTag(kTag_Sigma2_ResponderMRPParams), tlvReader));
        mExchangeCtxt->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteMRPConfig(mRemoteMRPConfig);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::SendSigma3()
{
    MATTER_TRACE_EVENT_SCOPE("SendSigma3", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    MutableByteSpan messageDigestSpan(mMessageDigest);
    System::PacketBufferHandle msg_R3;
    size_t data_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    size_t msg_r3_encrypted_len;

    uint8_t msg_salt[kIPKSize + kSHA256_Hash_Length];

    uint8_t sr3k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    size_t msg_r3_signed_len;

    P256ECDSASignature tbsData3Signature;

    chip::Platform::ScopedMemoryBuffer<uint8_t> icacBuf;
    MutableByteSpan icaCert;

    chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
    MutableByteSpan nocCert;

    ChipLogDetail(SecureChannel, "Sending Sigma3");

    VerifyOrExit(mEphemeralKey != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(icacBuf.Alloc(kMaxCHIPCertLength), err = CHIP_ERROR_NO_MEMORY);
    icaCert = MutableByteSpan{ icacBuf.Get(), kMaxCHIPCertLength };

    VerifyOrExit(nocBuf.Alloc(kMaxCHIPCertLength), err = CHIP_ERROR_NO_MEMORY);
    nocCert = MutableByteSpan{ nocBuf.Get(), kMaxCHIPCertLength };

    VerifyOrExit(mFabricsTable != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = mFabricsTable->FetchICACert(mFabricIndex, icaCert));
    SuccessOrExit(err = mFabricsTable->FetchNOCCert(mFabricIndex, nocCert));

    // Prepare Sigma3 TBS Data Blob
    msg_r3_signed_len = TLV::EstimateStructOverhead(icaCert.size(), nocCert.size(), kP256_PublicKey_Length, kP256_PublicKey_Length);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBSData(nocCert, icaCert, ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()),
                                         ByteSpan(mRemotePubKey, mRemotePubKey.Length()), msg_R3_Signed.Get(), msg_r3_signed_len));

    // Generate a signature
    err = mFabricsTable->SignWithOpKeypair(mFabricIndex, ByteSpan{ msg_R3_Signed.Get(), msg_r3_signed_len }, tbsData3Signature);
    SuccessOrExit(err);

    // Prepare Sigma3 TBE Data Blob
    msg_r3_encrypted_len = TLV::EstimateStructOverhead(nocCert.size(), icaCert.size(), tbsData3Signature.Length());

    VerifyOrExit(msg_R3_Encrypted.Alloc(msg_r3_encrypted_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderNOC), nocCert));
        if (!icaCert.empty())
        {
            SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderICAC), icaCert));
        }

        // We are now done with ICAC and NOC certs so we can release the memory.
        {
            icacBuf.Free();
            icaCert = MutableByteSpan{};

            nocBuf.Free();
            nocCert = MutableByteSpan{};
        }

        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(kTag_TBEData_Signature), tbsData3Signature,
                                               static_cast<uint32_t>(tbsData3Signature.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r3_encrypted_len = static_cast<size_t>(tlvWriter.GetLengthWritten());
    }

    // Generate S3K key
    {
        MutableByteSpan saltSpan(msg_salt);
        err = ConstructSaltSigma3(ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);

        HKDF_sha_crypto mHKDF;
        err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR3Info,
                                kKDFInfoLength, sr3k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        SuccessOrExit(err);
    }

    // Generated Encrypted data blob
    err = AES_CCM_encrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0, sr3k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES,
                          kTBEData3_Nonce, kTBEDataNonceLength, msg_R3_Encrypted.Get(),
                          msg_R3_Encrypted.Get() + msg_r3_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);
    SuccessOrExit(err);

    // Generate Sigma3 Msg
    data_len = TLV::EstimateStructOverhead(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, msg_r3_encrypted_len);

    msg_R3 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3));
        err = tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(TLV::ContextTag(1), msg_R3_Encrypted.Get(),
                                 static_cast<uint32_t>(msg_r3_encrypted_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3);
        SuccessOrExit(err);
    }

    err = mCommissioningHash.AddData(ByteSpan{ msg_R3->Start(), msg_R3->DataLength() });
    SuccessOrExit(err);

    // Call delegate to send the Msg3 to peer
    err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma3, std::move(msg_R3),
                                     SendFlags(SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    ChipLogProgress(SecureChannel, "Sent Sigma3 msg");

    err = mCommissioningHash.Finish(messageDigestSpan);
    SuccessOrExit(err);

    mState = State::kSentSigma3;

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        mState = State::kInitialized;
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigma3(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_EVENT_SCOPE("HandleSigma3", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    MutableByteSpan messageDigestSpan(mMessageDigest);
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf   = msg->Start();
    const uint16_t bufLen = msg->DataLength();

    constexpr size_t kCaseOverheadForFutureTbeData = 128;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    size_t msg_r3_encrypted_len          = 0;
    size_t msg_r3_encrypted_len_with_tag = 0;
    size_t max_msg_r3_signed_enc_len;
    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    size_t msg_r3_signed_len;

    uint8_t sr3k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    P256ECDSASignature tbsData3Signature;

    NodeId initiatorNodeId;
    P256PublicKey initiatorPublicKey;

    ByteSpan initiatorNOC;
    ByteSpan initiatorICAC;

    uint8_t msg_salt[kIPKSize + kSHA256_Hash_Length];

    ChipLogProgress(SecureChannel, "Received Sigma3 msg");

    VerifyOrExit(mEphemeralKey != nullptr, err = CHIP_ERROR_INTERNAL);

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Fetch encrypted data
    max_msg_r3_signed_enc_len = TLV::EstimateStructOverhead(Credentials::kMaxCHIPCertLength, Credentials::kMaxCHIPCertLength,
                                                            tbsData3Signature.Length(), kCaseOverheadForFutureTbeData);

    SuccessOrExit(err = tlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_Sigma3_Encrypted3)));

    msg_r3_encrypted_len_with_tag = tlvReader.GetLength();

    // Validate we did not receive a buffer larger than legal
    VerifyOrExit(msg_r3_encrypted_len_with_tag <= max_msg_r3_signed_enc_len, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrExit(msg_r3_encrypted_len_with_tag > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, err = CHIP_ERROR_INVALID_TLV_ELEMENT);

    VerifyOrExit(msg_R3_Encrypted.Alloc(msg_r3_encrypted_len_with_tag), err = CHIP_ERROR_NO_MEMORY);
    SuccessOrExit(err = tlvReader.GetBytes(msg_R3_Encrypted.Get(), static_cast<uint32_t>(msg_r3_encrypted_len_with_tag)));
    msg_r3_encrypted_len = msg_r3_encrypted_len_with_tag - CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

    // Step 1
    {
        MutableByteSpan saltSpan(msg_salt);
        err = ConstructSaltSigma3(ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);

        HKDF_sha_crypto mHKDF;
        err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR3Info,
                                kKDFInfoLength, sr3k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        SuccessOrExit(err);
    }

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, bufLen }));

    // Step 2 - Decrypt data blob
    SuccessOrExit(err = AES_CCM_decrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0,
                                        msg_R3_Encrypted.Get() + msg_r3_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, sr3k,
                                        CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, kTBEData3_Nonce, kTBEDataNonceLength,
                                        msg_R3_Encrypted.Get()));

    decryptedDataTlvReader.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
    containerType = TLV::kTLVType_Structure;
    SuccessOrExit(err = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = decryptedDataTlvReader.EnterContainer(containerType));

    SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_SenderNOC)));
    SuccessOrExit(err = decryptedDataTlvReader.Get(initiatorNOC));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    if (TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_SenderICAC)
    {
        VerifyOrExit(decryptedDataTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
        SuccessOrExit(err = decryptedDataTlvReader.Get(initiatorICAC));
        SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_Signature)));
    }

    // Step 5/6
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    SuccessOrExit(err = ValidatePeerIdentity(initiatorNOC, initiatorICAC, initiatorNodeId, initiatorPublicKey));
    mPeerNodeId = initiatorNodeId;

    // Step 4 - Construct Sigma3 TBS Data
    msg_r3_signed_len = TLV::EstimateStructOverhead(sizeof(uint16_t), initiatorNOC.size(), initiatorICAC.size(),
                                                    kP256_PublicKey_Length, kP256_PublicKey_Length);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBSData(initiatorNOC, initiatorICAC, ByteSpan(mRemotePubKey, mRemotePubKey.Length()),
                                         ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()), msg_R3_Signed.Get(),
                                         msg_r3_signed_len));

    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_Signature, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tbsData3Signature.Capacity() >= decryptedDataTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    tbsData3Signature.SetLength(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(tbsData3Signature, tbsData3Signature.Length()));

    // TODO - Validate message signature prior to validating the received operational credentials.
    //        The op cert check requires traversal of cert chain, that is a more expensive operation.
    //        If message signature check fails, the cert chain check will be unnecessary, but with the
    //        current flow of code, a malicious node can trigger a DoS style attack on the device.
    //        The same change should be made in Sigma2 processing.
    // Step 7 - Validate Signature
#ifdef ENABLE_HSM_ECDSA_VERIFY
    {
        P256PublicKeyHSM initiatorPublicKeyHSM;
        memcpy(Uint8::to_uchar(initiatorPublicKeyHSM), initiatorPublicKey.Bytes(), initiatorPublicKey.Length());
        SuccessOrExit(
            err = initiatorPublicKeyHSM.ECDSA_validate_msg_signature(msg_R3_Signed.Get(), msg_r3_signed_len, tbsData3Signature));
    }
#else
    SuccessOrExit(err = initiatorPublicKey.ECDSA_validate_msg_signature(msg_R3_Signed.Get(), msg_r3_signed_len, tbsData3Signature));
#endif

    SuccessOrExit(err = mCommissioningHash.Finish(messageDigestSpan));

    // Retrieve peer CASE Authenticated Tags (CATs) from peer's NOC.
    {
        SuccessOrExit(err = ExtractCATsFromOpCert(initiatorNOC, mPeerCATs));
    }

    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err2 = mSessionResumptionStorage->Save(GetPeer(), mNewResumptionId, mSharedSecret, mPeerCATs);
        if (err2 != CHIP_NO_ERROR)
            ChipLogError(SecureChannel, "Unable to save session resumption state: %" CHIP_ERROR_FORMAT, err2.Format());
    }

    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    mState = State::kFinished;
    Finish();

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::ConstructSaltSigma2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
                                            MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    bbuf.Put(rand.data(), kSigmaParamRandomNumberSize);
    bbuf.Put(pubkey, pubkey.Length());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);
    salt = salt.SubSpan(0, saltWritten);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigma3(const ByteSpan & ipk, MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);
    salt = salt.SubSpan(0, saltWritten);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSigmaResumeKey(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                                const ByteSpan & skInfo, const ByteSpan & nonce, MutableByteSpan & resumeKey)
{
    VerifyOrReturnError(resumeKey.size() >= CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    constexpr size_t saltSize = kSigmaParamRandomNumberSize + SessionResumptionStorage::kResumptionIdSize;
    uint8_t salt[saltSize];

    memset(salt, 0, saltSize);
    Encoding::LittleEndian::BufferWriter bbuf(salt, saltSize);

    bbuf.Put(initiatorRandom.data(), initiatorRandom.size());
    bbuf.Put(resumptionID.data(), resumptionID.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);

    HKDF_sha_crypto mHKDF;
    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), salt, saltWritten, skInfo.data(), skInfo.size(),
                                           resumeKey.data(), CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES));
    resumeKey.reduce_size(CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::GenerateSigmaResumeMIC(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                               const ByteSpan & skInfo, const ByteSpan & nonce, MutableByteSpan & resumeMIC)
{
    VerifyOrReturnError(resumeMIC.size() >= CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t srk[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    MutableByteSpan resumeKey(srk);

    ReturnErrorOnFailure(ConstructSigmaResumeKey(initiatorRandom, resumptionID, skInfo, nonce, resumeKey));

    ReturnErrorOnFailure(AES_CCM_encrypt(nullptr, 0, nullptr, 0, resumeKey.data(), resumeKey.size(), nonce.data(), nonce.size(),
                                         nullptr, resumeMIC.data(), CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
    resumeMIC.reduce_size(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidateSigmaResumeMIC(const ByteSpan & resumeMIC, const ByteSpan & initiatorRandom,
                                               const ByteSpan & resumptionID, const ByteSpan & skInfo, const ByteSpan & nonce)
{
    VerifyOrReturnError(resumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t srk[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    MutableByteSpan resumeKey(srk);

    ReturnErrorOnFailure(ConstructSigmaResumeKey(initiatorRandom, resumptionID, skInfo, nonce, resumeKey));

    ReturnErrorOnFailure(AES_CCM_decrypt(nullptr, 0, nullptr, 0, resumeMIC.data(), resumeMIC.size(), resumeKey.data(),
                                         resumeKey.size(), nonce.data(), nonce.size(), nullptr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidatePeerIdentity(const ByteSpan & peerNOC, const ByteSpan & peerICAC, NodeId & peerNodeId,
                                             Crypto::P256PublicKey & peerPublicKey)
{
    ReturnErrorCodeIf(mFabricsTable == nullptr, CHIP_ERROR_INCORRECT_STATE);
    const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(mFabricIndex);
    ReturnErrorCodeIf(fabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(SetEffectiveTime());

    CompressedFabricId unused;
    FabricId peerFabricId;
    ReturnErrorOnFailure(mFabricsTable->VerifyCredentials(mFabricIndex, peerNOC, peerICAC, mValidContext, unused, peerFabricId,
                                                          peerNodeId, peerPublicKey));
    VerifyOrReturnError(fabricInfo->GetFabricId() == peerFabricId, CHIP_ERROR_INVALID_CASE_PARAMETER);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBSData(const ByteSpan & senderNOC, const ByteSpan & senderICAC, const ByteSpan & senderPubKey,
                                         const ByteSpan & receiverPubKey, uint8_t * tbsData, size_t & tbsDataLen)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    enum
    {
        kTag_TBSData_SenderNOC      = 1,
        kTag_TBSData_SenderICAC     = 2,
        kTag_TBSData_SenderPubKey   = 3,
        kTag_TBSData_ReceiverPubKey = 4,
    };

    tlvWriter.Init(tbsData, tbsDataLen);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_SenderNOC), senderNOC));
    if (!senderICAC.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_SenderICAC), senderICAC));
    }
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_SenderPubKey), senderPubKey));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_ReceiverPubKey), receiverPubKey));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    tbsDataLen = static_cast<size_t>(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SetEffectiveTime()
{
    System::Clock::Milliseconds64 currentUnixTimeMS;
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);

    if (err == CHIP_NO_ERROR)
    {
        // If the system has given us a wall clock time, we must use it or
        // fail.  Conversion failures here are therefore always an error.
        System::Clock::Seconds32 currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
        ReturnErrorOnFailure(mValidContext.SetEffectiveTimeFromUnixTime<CurrentChipEpochTime>(currentUnixTime));
    }
    else
    {
        // If we don't have wall clock time, the spec dictates that we should
        // fall back to Last Known Good Time.  Ultimately, the calling application's
        // validity policy will determine whether this is permissible.
        System::Clock::Seconds32 lastKnownGoodChipEpochTime;
        ChipLogError(SecureChannel,
                     "The device does not support GetClock_RealTimeMS() API: %" CHIP_ERROR_FORMAT
                     ".  Falling back to Last Known Good UTC Time",
                     err.Format());
        VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
        err = mFabricsTable->GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime);
        if (err != CHIP_NO_ERROR)
        {
            // If we have no time available, the Validity Policy will
            // determine what to do.
            ChipLogError(SecureChannel, "Failed to retrieve Last Known Good UTC Time");
        }
        else
        {
            mValidContext.SetEffectiveTime<LastKnownGoodChipEpochTime>(lastKnownGoodChipEpochTime);
        }
    }
    return CHIP_NO_ERROR;
}

void CASESession::OnSuccessStatusReport()
{
    ChipLogProgress(SecureChannel, "Success status report received. Session was established");

    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err2 = mSessionResumptionStorage->Save(GetPeer(), mNewResumptionId, mSharedSecret, mPeerCATs);
        if (err2 != CHIP_NO_ERROR)
            ChipLogError(SecureChannel, "Unable to save session resumption state: %" CHIP_ERROR_FORMAT, err2.Format());
    }

    switch (mState)
    {
    case State::kSentSigma3:
        mState = State::kFinished;
        break;
    case State::kSentSigma2Resume:
        mState = State::kFinishedViaResume;
        break;
    default:
        VerifyOrDie(false && "Reached invalid internal state keeping in CASE session");
        break;
    }

    Finish();
}

CHIP_ERROR CASESession::OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (protocolCode)
    {
    case kProtocolCodeInvalidParam:
        err = CHIP_ERROR_INVALID_CASE_PARAMETER;
        break;

    case kProtocolCodeNoSharedRoot:
        err = CHIP_ERROR_NO_SHARED_TRUSTED_ROOT;
        break;

    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    };
    mState = State::kInitialized;
    ChipLogError(SecureChannel, "Received error (protocol code %d) during pairing process: %" CHIP_ERROR_FORMAT, protocolCode,
                 err.Format());
    return err;
}

CHIP_ERROR CASESession::ParseSigma1(TLV::ContiguousBufferTLVReader & tlvReader, ByteSpan & initiatorRandom,
                                    uint16_t & initiatorSessionId, ByteSpan & destinationId, ByteSpan & initiatorEphPubKey,
                                    bool & resumptionRequested, ByteSpan & resumptionId, ByteSpan & initiatorResumeMIC)
{
    using namespace TLV;

    constexpr uint8_t kInitiatorRandomTag    = 1;
    constexpr uint8_t kInitiatorSessionIdTag = 2;
    constexpr uint8_t kDestinationIdTag      = 3;
    constexpr uint8_t kInitiatorPubKeyTag    = 4;
    constexpr uint8_t kInitiatorMRPParamsTag = 5;
    constexpr uint8_t kResumptionIDTag       = 6;
    constexpr uint8_t kResume1MICTag         = 7;

    TLVType containerType = kTLVType_Structure;
    ReturnErrorOnFailure(tlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kInitiatorRandomTag)));
    ReturnErrorOnFailure(tlvReader.GetByteView(initiatorRandom));
    VerifyOrReturnError(initiatorRandom.size() == kSigmaParamRandomNumberSize, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kInitiatorSessionIdTag)));
    ReturnErrorOnFailure(tlvReader.Get(initiatorSessionId));

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kDestinationIdTag)));
    ReturnErrorOnFailure(tlvReader.GetByteView(destinationId));
    VerifyOrReturnError(destinationId.size() == kSHA256_Hash_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kInitiatorPubKeyTag)));
    ReturnErrorOnFailure(tlvReader.GetByteView(initiatorEphPubKey));
    VerifyOrReturnError(initiatorEphPubKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Optional members start here.
    CHIP_ERROR err = tlvReader.Next();
    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == ContextTag(kInitiatorMRPParamsTag))
    {
        ReturnErrorOnFailure(DecodeMRPParametersIfPresent(TLV::ContextTag(kInitiatorMRPParamsTag), tlvReader));
        mExchangeCtxt->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteMRPConfig(mRemoteMRPConfig);
        err = tlvReader.Next();
    }

    bool resumptionIDTagFound = false;
    bool resume1MICTagFound   = false;

    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == ContextTag(kResumptionIDTag))
    {
        resumptionIDTagFound = true;
        ReturnErrorOnFailure(tlvReader.GetByteView(resumptionId));
        VerifyOrReturnError(resumptionId.size() == SessionResumptionStorage::kResumptionIdSize, CHIP_ERROR_INVALID_CASE_PARAMETER);
        err = tlvReader.Next();
    }

    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == ContextTag(kResume1MICTag))
    {
        resume1MICTagFound = true;
        ReturnErrorOnFailure(tlvReader.GetByteView(initiatorResumeMIC));
        VerifyOrReturnError(initiatorResumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_INVALID_CASE_PARAMETER);
        err = tlvReader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        // We ran out of struct members, but that's OK, because they were optional.
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(tlvReader.ExitContainer(containerType));

    if (resumptionIDTagFound && resume1MICTagFound)
    {
        resumptionRequested = true;
    }
    else if (!resumptionIDTagFound && !resume1MICTagFound)
    {
        resumptionRequested = false;
    }
    else
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidateReceivedMessage(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                const System::PacketBufferHandle & msg)
{
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // mExchangeCtxt can be nullptr if this is the first message (CASE_Sigma1) received by CASESession
    // via UnsolicitedMessageHandler. The exchange context is allocated by exchange manager and provided
    // to the handler (CASESession object).
    if (mExchangeCtxt != nullptr)
    {
        if (mExchangeCtxt != ec)
        {
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    else
    {
        mExchangeCtxt = ec;
        mExchangeCtxt->SetResponseTimeout(kSigma_Response_Timeout + mExchangeCtxt->GetSessionHandle()->GetAckTimeout());
    }

    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && msg)
{
    CHIP_ERROR err                            = ValidateReceivedMessage(ec, payloadHeader, msg);
    Protocols::SecureChannel::MsgType msgType = static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType());
    SuccessOrExit(err);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (mStopHandshakeAtState.HasValue() && mState == mStopHandshakeAtState.Value())
    {
        mStopHandshakeAtState = Optional<State>::Missing();
        // For testing purposes we are trying to stop a successful CASESession from happening by dropping part of the
        // handshake in the middle. We are trying to keep both sides of the CASESession establishment in an active
        // pending state. In order to keep this side open we have to tell the exchange context that we will send an
        // async message.
        //
        // Should you need to resume the CASESession, you could theoretically pass along the msg to a callback that gets
        // registered when setting mStopHandshakeAtState.
        mExchangeCtxt->WillSendMessage();
        return CHIP_NO_ERROR;
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

#if CHIP_CONFIG_SLOW_CRYPTO
    if (msgType == Protocols::SecureChannel::MsgType::CASE_Sigma1 || msgType == Protocols::SecureChannel::MsgType::CASE_Sigma2 ||
        msgType == Protocols::SecureChannel::MsgType::CASE_Sigma2Resume ||
        msgType == Protocols::SecureChannel::MsgType::CASE_Sigma3)
    {
        SuccessOrExit(mExchangeCtxt->FlushAcks());
    }
#endif // CHIP_CONFIG_SLOW_CRYPTO

    // By default, CHIP_ERROR_INVALID_MESSAGE_TYPE is returned if in the current state
    // a message handler is not defined for the received message type.
    err = CHIP_ERROR_INVALID_MESSAGE_TYPE;

    switch (mState)
    {
    case State::kInitialized:
        if (msgType == Protocols::SecureChannel::MsgType::CASE_Sigma1)
        {
            err = HandleSigma1_and_SendSigma2(std::move(msg));
        }
        break;
    case State::kSentSigma1:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma2:
            err = HandleSigma2_and_SendSigma3(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case State::kSentSigma1Resume:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma2:
            err = HandleSigma2_and_SendSigma3(std::move(msg));
            break;

        case Protocols::SecureChannel::MsgType::CASE_Sigma2Resume:
            err = HandleSigma2Resume(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case State::kSentSigma2:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma3:
            err = HandleSigma3(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case State::kSentSigma3:
    case State::kSentSigma2Resume:
        if (msgType == Protocols::SecureChannel::MsgType::StatusReport)
        {
            err = HandleStatusReport(std::move(msg), /* successExpected*/ true);
        }
        break;
    default:
        // Return the default error that was set above
        break;
    };

exit:

    if (err == CHIP_ERROR_INVALID_MESSAGE_TYPE)
    {
        ChipLogError(SecureChannel, "Received message (type %d) cannot be handled in %d state.", to_underlying(msgType),
                     to_underlying(mState));
    }

    // Call delegate to indicate session establishment failure.
    if (err != CHIP_NO_ERROR)
    {
        // Discard the exchange so that Clear() doesn't try aborting it.  The
        // exchange will handle that.
        DiscardExchange();
        AbortPendingEstablish(err);
    }
    return err;
}

} // namespace chip
