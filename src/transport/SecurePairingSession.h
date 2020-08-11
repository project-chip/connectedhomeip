/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the CHIP SPAKE2P Session object that provides
 *      APIs for constructing spake2p messages and establishing encryption
 *      keys.
 *
 */

#ifndef __SECUREPAIRINGSESSION_H__
#define __SECUREPAIRINGSESSION_H__

#include <core/ReferenceCounted.h>
#include <crypto/CHIPCryptoPAL.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSession.h>

namespace chip {

extern const char * kSpake2pI2RSessionInfo;
extern const char * kSpake2pR2ISessionInfo;

using namespace Crypto;

class DLL_EXPORT SecurePairingSessionDelegate : public ReferenceCounted<SecurePairingSessionDelegate>
{
public:
    /**
     * @brief
     *   Called when pairing session generates a new message that should be sent to peer.
     *
     * @param msgBuf the new message that should be sent to the peer
     */
    virtual CHIP_ERROR OnNewMessageForPeer(uint8_t msgType, System::PacketBuffer * msgBuf) { return CHIP_NO_ERROR; }

    /**
     * @brief
     *   Called when pairing fails with an error
     *
     * @param error error code
     */
    virtual void OnPairingError(CHIP_ERROR error) {}

    /**
     * @brief
     *   Called when the pairing is complete and the new secure session has been established
     *
     */
    virtual void OnPairingComplete() {}

    virtual ~SecurePairingSessionDelegate() {}
};

class DLL_EXPORT SecurePairingSession
{
public:
    SecurePairingSession(void);
    SecurePairingSession(SecurePairingSession &&)      = default;
    SecurePairingSession(const SecurePairingSession &) = delete;
    SecurePairingSession & operator=(const SecurePairingSession &) = delete;
    SecurePairingSession & operator=(SecurePairingSession &&) = default;

    ~SecurePairingSession(void);

    /**
     * @brief
     *   Initialize using setup PIN code and wait for pairing requests.
     *
     * @param mySetUpPINCode  Setup PIN code of the local device
     * @param delegate        Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const unsigned char * salt, size_t saltLen,
                              SecurePairingSessionDelegate * delegate);

    /**
     * @brief
     *   Create a pairing request using peer's setup PIN code.
     *
     * @param peerSetUpPINCode Setup PIN code of the peer device
     * @param delegate         Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR Pair(uint32_t peerSetUpPINCode, uint32_t pbkdf2IterCount, const unsigned char * salt, size_t saltLen,
                    SecurePairingSessionDelegate * delegate);

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param info        Information string used for key derivation
     * @param info_len    Length of info string
     * @param session     Referene to the sescure session that will be
     *                    initialized once pairing is complete
     * @return CHIP_ERROR The result of session derivation
     */
    CHIP_ERROR DeriveSecureSession(const unsigned char * info, size_t info_len, SecureSession * session);

    /**
     * @brief
     *   Handler for peer's messages, exchanged during pairing handshake.
     *
     * @param msg         Message sent by the peer
     * @return CHIP_ERROR The result of message processing
     */
    CHIP_ERROR HandlePeerMessage(const MessageHeader & header, System::PacketBuffer * msg);

private:
    CHIP_ERROR Init(uint32_t setupCode, uint32_t pbkdf2IterCount, const unsigned char * salt, size_t saltLen,
                    SecurePairingSessionDelegate * delegate);

    CHIP_ERROR HandleCompute_pA(const MessageHeader & header, System::PacketBuffer * msg);
    CHIP_ERROR HandleCompute_pB_cB(const MessageHeader & header, System::PacketBuffer * msg);
    CHIP_ERROR HandleCompute_cA(const MessageHeader & header, System::PacketBuffer * msg);

    static constexpr size_t kSpake2p_WS_Length = kP256_FE_Length + 8;

    enum Spake2pMsgType : uint8_t
    {
        kSpake2pCompute_pA    = 0,
        kSpake2pCompute_pB_cB = 1,
        kSpake2pCompute_cA    = 2,
        kSpake2pMsgTypeMax    = 3,
    };

    SecurePairingSessionDelegate * mDelegate = nullptr;

    Spake2pMsgType mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;

    Spake2p_P256_SHA256_HKDF_HMAC mSpake2p;

    uint8_t mPoint[kMAX_Point_Length];

    /* w0s and w1s */
    uint8_t mWS[2][kSpake2p_WS_Length];

    uint8_t mKe[kMAX_Hash_Length];

    size_t mKeLen = sizeof(mKe);

    bool mPairingComplete = false;
};

} // namespace chip

#endif // __SECUREPAIRINGSESSION_H__
