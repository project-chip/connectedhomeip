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
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#pragma once

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif
#include <credentials/CertificateValidityPolicy.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/Base64.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/secure_channel/CASEDestinationId.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/PairingSession.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <protocols/secure_channel/SessionResumptionStorage.h>
#include <system/SystemPacketBuffer.h>
#include <transport/CryptoContext.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
#define CASE_EPHEMERAL_KEY 0xCA5EECD0
#endif

// TODO: temporary derive from Messaging::UnsolicitedMessageHandler, actually the CASEServer should be the umh, it will be fixed
// when implementing concurrent CASE session.
class DLL_EXPORT CASESession : public Messaging::UnsolicitedMessageHandler,
                               public Messaging::ExchangeDelegate,
                               public PairingSession
{
public:
    ~CASESession() override;

    Transport::SecureSession::Type GetSecureSessionType() const override { return Transport::SecureSession::Type::kCASE; }
    ScopedNodeId GetPeer() const override { return ScopedNodeId(mPeerNodeId, GetFabricIndex()); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(mLocalNodeId, GetFabricIndex()); }
    CATValues GetPeerCATs() const override { return mPeerCATs; };

    /**
     * @brief
     *   Initialize using configured fabrics and wait for session establishment requests.
     *
     * @param sessionManager                session manager from which to allocate a secure session object
     * @param fabricTable                   Table of fabrics that are currently configured on the device
     * @param policy                        Optional application-provided certificate validity policy
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR ListenForSessionEstablishment(
        SessionManager & sessionManager, FabricTable * fabricTable, SessionResumptionStorage * sessionResumptionStorage,
        Credentials::CertificateValidityPolicy * policy, SessionEstablishmentDelegate * delegate,
        Optional<ReliableMessageProtocolConfig> mrpConfig = Optional<ReliableMessageProtocolConfig>::Missing());

    /**
     * @brief
     *   Create and send session establishment request using device's operational credentials.
     *
     * @param sessionManager                session manager from which to allocate a secure session object
     * @param fabricTable                   The fabric table to be used for connecting with the peer
     * @param fabricIndex                   The index of the fabric to be used for connecting with the peer
     * @param peerNodeId                    Node id of the peer node
     * @param exchangeCtxt                  The exchange context to send and receive messages with the peer
     * @param policy                        Optional application-provided certificate validity policy
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR
    EstablishSession(SessionManager & sessionManager, FabricTable * fabricTable, FabricIndex fabricIndex, NodeId peerNodeId,
                     Messaging::ExchangeContext * exchangeCtxt, SessionResumptionStorage * sessionResumptionStorage,
                     Credentials::CertificateValidityPolicy * policy, SessionEstablishmentDelegate * delegate,
                     Optional<ReliableMessageProtocolConfig> mrpConfig = Optional<ReliableMessageProtocolConfig>::Missing());

    /**
     * @brief Set the Group Data Provider which will be used to look up IPKs
     *
     * The GroupDataProvider set MUST have key sets available through `GetIpkKeySet` method
     * for the FabricIndex that is associated with the CASESession's FabricInfo.
     *
     * @param groupDataProvider - Pointer to the group data provider (if nullptr, will error at start of
     *                            establishment, not here).
     */
    void SetGroupDataProvider(Credentials::GroupDataProvider * groupDataProvider) { mGroupDataProvider = groupDataProvider; }

    /**
     * Parse a sigma1 message.  This function will return success only if the
     * message passes schema checks.  Specifically:
     *   * The tags come in order.
     *   * The required tags are present.
     *   * The values for the tags that are present satisfy schema requirements
     *     (e.g. constraints on octet string lengths)
     *   * Either resumptionID and initiatorResume1MIC are both present or both
     *     absent.
     *
     * On success, the initiatorRandom, initiatorSessionId, destinationId,
     * initiatorEphPubKey outparams will be set to the corresponding values in
     * the message.
     *
     * On success, either the resumptionRequested outparam will be set to true
     * and the  resumptionID and initiatorResumeMIC outparams will be set to
     * valid values, or the resumptionRequested outparam will be set to false.
     */
    CHIP_ERROR ParseSigma1(TLV::ContiguousBufferTLVReader & tlvReader, ByteSpan & initiatorRandom, uint16_t & initiatorSessionId,
                           ByteSpan & destinationId, ByteSpan & initiatorEphPubKey, bool & resumptionRequested,
                           ByteSpan & resumptionId, ByteSpan & initiatorResumeMIC);

    /**
     * @brief
     *   Derive a secure session from the established session. The API will return error if called before session is established.
     *
     * @param session     Reference to the secure session that will be initialized once session establishment is complete
     * @return CHIP_ERROR The result of session derivation
     */
    CHIP_ERROR DeriveSecureSession(CryptoContext & session) const override;

    //// UnsolicitedMessageHandler Implementation ////
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    //// ExchangeDelegate Implementation ////
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return SessionEstablishmentExchangeDispatch::Instance(); }

    //// SessionDelegate ////
    void OnSessionReleased() override;

    FabricIndex GetFabricIndex() const { return mFabricIndex; }

    // TODO: remove Clear, we should create a new instance instead reset the old instance.
    /** @brief This function zeroes out and resets the memory used by the object.
     **/
    void Clear();

    void InvalidateIfPendingEstablishment();

private:
    class CASESessionFabricDelegate final : public chip::FabricTable::Delegate
    {
    public:
        CASESessionFabricDelegate(CASESession * caseSession) : mCASESession(caseSession) {}

        void OnFabricDeletedFromStorage(FabricTable & fabricTable, FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            (void) fabricIndex;
            mCASESession->InvalidateIfPendingEstablishment();
        }

        void OnFabricRetrievedFromStorage(FabricTable & fabricTable, FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            (void) fabricIndex;
        }

        void OnFabricPersistedToStorage(FabricTable & fabricTable, FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            (void) fabricIndex;
        }

        void OnFabricNOCUpdated(chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex) override
        {
            (void) fabricTable;
            (void) fabricIndex;
            mCASESession->InvalidateIfPendingEstablishment();
        }

    private:
        CASESession * mCASESession;
    };

    enum class State : uint8_t
    {
        kInitialized       = 0,
        kSentSigma1        = 1,
        kSentSigma2        = 2,
        kSentSigma3        = 3,
        kSentSigma1Resume  = 4,
        kSentSigma2Resume  = 5,
        kFinished          = 6,
        kFinishedViaResume = 7,
    };

    CHIP_ERROR Init(SessionManager & sessionManager, Credentials::CertificateValidityPolicy * policy,
                    SessionEstablishmentDelegate * delegate);

    // On success, sets mIpk to the correct value for outgoing Sigma1 based on internal state
    CHIP_ERROR RecoverInitiatorIpk();
    // On success, sets locally maching mFabricInfo in internal state to the entry matched by
    // destinationId/initiatorRandom from processing of Sigma1, and sets mIpk to the right IPK.
    CHIP_ERROR FindLocalNodeFromDestionationId(const ByteSpan & destinationId, const ByteSpan & initiatorRandom);

    CHIP_ERROR SendSigma1();
    CHIP_ERROR HandleSigma1_and_SendSigma2(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigma1(System::PacketBufferHandle && msg);
    CHIP_ERROR TryResumeSession(SessionResumptionStorage::ConstResumptionIdView resumptionId, ByteSpan resume1MIC,
                                ByteSpan initiatorRandom);
    CHIP_ERROR SendSigma2();
    CHIP_ERROR HandleSigma2_and_SendSigma3(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigma2(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigma2Resume(System::PacketBufferHandle && msg);
    CHIP_ERROR SendSigma3();
    CHIP_ERROR HandleSigma3(System::PacketBufferHandle && msg);

    CHIP_ERROR SendSigma2Resume();

    CHIP_ERROR ConstructSaltSigma2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
                                   MutableByteSpan & salt);
    CHIP_ERROR ValidatePeerIdentity(const ByteSpan & peerNOC, const ByteSpan & peerICAC, NodeId & peerNodeId,
                                    Crypto::P256PublicKey & peerPublicKey);
    CHIP_ERROR ConstructTBSData(const ByteSpan & senderNOC, const ByteSpan & senderICAC, const ByteSpan & senderPubKey,
                                const ByteSpan & receiverPubKey, uint8_t * tbsData, size_t & tbsDataLen);
    CHIP_ERROR ConstructSaltSigma3(const ByteSpan & ipk, MutableByteSpan & salt);

    CHIP_ERROR ConstructSigmaResumeKey(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID, const ByteSpan & skInfo,
                                       const ByteSpan & nonce, MutableByteSpan & resumeKey);

    CHIP_ERROR GenerateSigmaResumeMIC(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID, const ByteSpan & skInfo,
                                      const ByteSpan & nonce, MutableByteSpan & resumeMIC);
    CHIP_ERROR ValidateSigmaResumeMIC(const ByteSpan & resumeMIC, const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                      const ByteSpan & skInfo, const ByteSpan & nonce);

    void OnSuccessStatusReport() override;
    CHIP_ERROR OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode) override;

    void AbortPendingEstablish(CHIP_ERROR err);

    CHIP_ERROR GetHardcodedTime();

    CHIP_ERROR SetEffectiveTime();

    CHIP_ERROR ValidateReceivedMessage(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                       const System::PacketBufferHandle & msg);

    Crypto::Hash_SHA256_stream mCommissioningHash;
    Crypto::P256PublicKey mRemotePubKey;
#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
    Crypto::P256KeypairHSM mEphemeralKey;
#else
    Crypto::P256Keypair mEphemeralKey;
#endif
    Crypto::P256ECDHDerivedSecret mSharedSecret;
    Credentials::ValidationContext mValidContext;
    Credentials::GroupDataProvider * mGroupDataProvider = nullptr;

    uint8_t mMessageDigest[Crypto::kSHA256_Hash_Length];
    uint8_t mIPK[kIPKSize];

    SessionResumptionStorage * mSessionResumptionStorage = nullptr;

    FabricTable * mFabricsTable = nullptr;
    FabricIndex mFabricIndex    = kUndefinedFabricIndex;
    NodeId mPeerNodeId          = kUndefinedNodeId;
    NodeId mLocalNodeId         = kUndefinedNodeId;
    CATValues mPeerCATs;

    SessionResumptionStorage::ResumptionIdStorage mResumeResumptionId; // ResumptionId which is used to resume this session
    SessionResumptionStorage::ResumptionIdStorage mNewResumptionId;    // ResumptionId which is stored to resume future session
    // Sigma1 initiator random, maintained to be reused post-Sigma1, such as when generating Sigma2 S2RK key
    uint8_t mInitiatorRandom[kSigmaParamRandomNumberSize];

    CASESessionFabricDelegate mFabricDelegate{ this };
    State mState;
};

} // namespace chip
