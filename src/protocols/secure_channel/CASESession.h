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
#include <credentials/FabricTable.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/Base64.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <system/SystemPacketBuffer.h>
#include <transport/CryptoContext.h>
#include <transport/PairingSession.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

constexpr uint16_t kSigmaParamRandomNumberSize = 32;
constexpr uint16_t kTrustedRootIdSize          = Crypto::kSubjectKeyIdentifierLength;
constexpr uint16_t kMaxTrustedRootIds          = 5;

constexpr uint16_t kIPKSize = 16;

constexpr size_t kCASEResumptionIDSize = 16;

#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
#define CASE_EPHEMERAL_KEY 0xCA5EECD0
#endif

struct CASESessionCachable
{
    uint16_t mSharedSecretLen                              = 0;
    uint8_t mSharedSecret[Crypto::kMax_ECDH_Secret_Length] = { 0 };
    FabricIndex mLocalFabricIndex                          = 0;
    NodeId mPeerNodeId                                     = kUndefinedNodeId;
    CATValues mPeerCATs;
    uint8_t mResumptionId[kCASEResumptionIDSize] = { 0 };
    uint64_t mSessionSetupTimeStamp              = 0;
};

class DLL_EXPORT CASESession : public Messaging::ExchangeDelegate, public PairingSession
{
public:
    CASESession();
    CASESession(CASESession &&)      = default;
    CASESession(const CASESession &) = default;

    virtual ~CASESession();

    /**
     * @brief
     *   Initialize using configured fabrics and wait for session establishment requests.
     *
     * @param mySessionId                   Session ID to be assigned to the secure session on the peer node
     * @param fabrics                       Table of fabrics that are currently configured on the device
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR ListenForSessionEstablishment(
        uint16_t mySessionId, FabricTable * fabrics, SessionEstablishmentDelegate * delegate,
        Optional<ReliableMessageProtocolConfig> mrpConfig = Optional<ReliableMessageProtocolConfig>::Missing());

    /**
     * @brief
     *   Create and send session establishment request using device's operational credentials.
     *
     * @param peerAddress                   Address of peer with which to establish a session.
     * @param fabric                        The fabric that should be used for connecting with the peer
     * @param peerNodeId                    Node id of the peer node
     * @param mySessionId                   Session ID to be assigned to the secure session on the peer node
     * @param exchangeCtxt                  The exchange context to send and receive messages with the peer
     * @param delegate                      Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR
    EstablishSession(const Transport::PeerAddress peerAddress, FabricInfo * fabric, NodeId peerNodeId, uint16_t mySessionId,
                     Messaging::ExchangeContext * exchangeCtxt, SessionEstablishmentDelegate * delegate,
                     Optional<ReliableMessageProtocolConfig> mrpConfig = Optional<ReliableMessageProtocolConfig>::Missing());

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
     *   Derive a secure session from the established session. The API will return error
     *   if called before session is established.
     *
     * @param session     Reference to the secure session that will be
     *                    initialized once session establishment is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(CryptoContext & session, CryptoContext::SessionRole role) override;

    /**
     * @brief Serialize the CASESession to the given cachableSession data structure for secure pairing
     **/
    CHIP_ERROR ToCachable(CASESessionCachable & output);

    /**
     * @brief Reconstruct secure pairing class from the cachableSession data structure.
     **/
    CHIP_ERROR FromCachable(const CASESessionCachable & output);

    //// ExchangeDelegate Implementation ////
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return SessionEstablishmentExchangeDispatch::Instance(); }

    FabricIndex GetFabricIndex() const { return mFabricInfo != nullptr ? mFabricInfo->GetFabricIndex() : kUndefinedFabricIndex; }

    // TODO: remove Clear, we should create a new instance instead reset the old instance.
    /** @brief This function zeroes out and resets the memory used by the object.
     **/
    void Clear();

    /**
     * Parse the TLV for Sigma1 message.
     */
    CHIP_ERROR ParseSigma1();

private:
    enum State : uint8_t
    {
        kInitialized      = 0,
        kSentSigma1       = 1,
        kSentSigma2       = 2,
        kSentSigma3       = 3,
        kSentSigma2Resume = 4,
    };

    CHIP_ERROR Init(uint16_t mySessionId, SessionEstablishmentDelegate * delegate);

    CHIP_ERROR SendSigma1();
    CHIP_ERROR HandleSigma1_and_SendSigma2(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigma1(System::PacketBufferHandle && msg);
    CHIP_ERROR SendSigma2();
    CHIP_ERROR HandleSigma2_and_SendSigma3(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigma2(System::PacketBufferHandle && msg);
    CHIP_ERROR HandleSigma2Resume(System::PacketBufferHandle && msg);
    CHIP_ERROR SendSigma3();
    CHIP_ERROR HandleSigma3(System::PacketBufferHandle && msg);

    CHIP_ERROR SendSigma2Resume(const ByteSpan & initiatorRandom);

    CHIP_ERROR ConstructSaltSigma2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
                                   MutableByteSpan & salt);
    CHIP_ERROR Validate_and_RetrieveResponderID(const ByteSpan & responderNOC, const ByteSpan & responderICAC,
                                                Crypto::P256PublicKey & responderID);
    CHIP_ERROR ConstructTBSData(const ByteSpan & senderNOC, const ByteSpan & senderICAC, const ByteSpan & senderPubKey,
                                const ByteSpan & receiverPubKey, uint8_t * tbsData, size_t & tbsDataLen);
    CHIP_ERROR ConstructSaltSigma3(const ByteSpan & ipk, MutableByteSpan & salt);
    CHIP_ERROR RetrieveIPK(FabricId fabricId, MutableByteSpan & ipk);

    CHIP_ERROR ConstructSigmaResumeKey(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID, const ByteSpan & skInfo,
                                       const ByteSpan & nonce, MutableByteSpan & resumeKey);

    CHIP_ERROR GenerateSigmaResumeMIC(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID, const ByteSpan & skInfo,
                                      const ByteSpan & nonce, MutableByteSpan & resumeMIC);
    CHIP_ERROR ValidateSigmaResumeMIC(const ByteSpan & resumeMIC, const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                      const ByteSpan & skInfo, const ByteSpan & nonce);

    void OnSuccessStatusReport() override;
    CHIP_ERROR OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode) override;

    void CloseExchange();

    /**
     * Clear our reference to our exchange context pointer so that it can close
     * itself at some later time.
     */
    void DiscardExchange();

    CHIP_ERROR GetHardcodedTime();

    CHIP_ERROR SetEffectiveTime();

    CHIP_ERROR ValidateReceivedMessage(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                       System::PacketBufferHandle & msg);

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Crypto::Hash_SHA256_stream mCommissioningHash;
    Crypto::P256PublicKey mRemotePubKey;
#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
    Crypto::P256KeypairHSM mEphemeralKey;
#else
    Crypto::P256Keypair mEphemeralKey;
#endif
    Crypto::P256ECDHDerivedSecret mSharedSecret;
    Credentials::CertificateKeyId mTrustedRootId;
    Credentials::ValidationContext mValidContext;

    uint8_t mMessageDigest[Crypto::kSHA256_Hash_Length];
    uint8_t mIPK[kIPKSize];

    Messaging::ExchangeContext * mExchangeCtxt = nullptr;

    FabricTable * mFabricsTable = nullptr;
    FabricInfo * mFabricInfo    = nullptr;

    uint8_t mResumptionId[kCASEResumptionIDSize];
    // Sigma1 initiator random, maintained to be reused post-Sigma1, such as when generating Sigma2 S2RK key
    uint8_t mInitiatorRandom[kSigmaParamRandomNumberSize];

    State mState;

    uint8_t mLocalFabricIndex       = 0;
    uint64_t mSessionSetupTimeStamp = 0;

    Optional<ReliableMessageProtocolConfig> mLocalMRPConfig;

protected:
    bool mCASESessionEstablished = false;

    virtual ByteSpan * GetIPKList() const
    {
        // TODO: Remove this list. Replace it with an actual method to retrieve an IPK list (e.g. from a Crypto Store API)
        static uint8_t sIPKList[][kIPKSize] = {
            { 0 }, /* Corresponds to the FabricID for the Commissioning Example. All zeros. */
        };
        static ByteSpan ipkListSpan[] = { ByteSpan(sIPKList[0]) };
        return ipkListSpan;
    }
    virtual size_t GetIPKListEntries() const { return 1; }

    void SetSessionTimeStamp(uint64_t timestamp) { mSessionSetupTimeStamp = timestamp; }
};

} // namespace chip
