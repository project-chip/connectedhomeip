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

/**
 *    @file
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <protocols/secure_channel/Constants.h>
#include <support/Base64.h>
#include <system/SystemPacketBuffer.h>
#include <transport/PeerConnectionState.h>
#include <transport/SecureSession.h>
#include <transport/SessionEstablishmentDelegate.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

using namespace Crypto;

class DLL_EXPORT CASESession
{
public:
    CASESession();
    CASESession(CASESession &&)      = default;
    CASESession(const CASESession &) = default;
    CASESession & operator=(const CASESession &) = default;
    CASESession & operator=(CASESession &&) = default;

    virtual ~CASESession();

    /**
     * @brief
     *   Initialize using operational credentials code and wait for session establishment requests.
     *
     * @param myNodeId        Node id of local node
     * @param myKeyId         Key ID to be assigned to the secure session on the peer node
     * @param delegate        Callback object
     *
     * @return CHIP_ERROR     The result of initialization
     */
    CHIP_ERROR WaitForSessionEstablishment(NodeId myNodeId, uint16_t myKeyId, SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Create and send session establishment request using device's operational credentials.
     *
     * @param peerAddress      Address of peer with which to establish a session.
     * @param myNodeId         Node id of local node
     * @param peerNodeId       Node id of the peer node
     * @param myKeyId          Key ID to be assigned to the secure session on the peer node
     * @param delegate         Callback object
     *
     * @return CHIP_ERROR      The result of initialization
     */
    CHIP_ERROR EstablishSession(const Transport::PeerAddress peerAddress, NodeId myNodeId, NodeId peerNodeId, uint16_t myKeyId,
                                SessionEstablishmentDelegate * delegate);

    /**
     * @brief
     *   Derive a secure session from the established session. The API will return error
     *   if called before session is established.
     *
     * @param session     Reference to the secure session that will be
     *                    initialized once session establishment is complete
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(SecureSession & session);

    /**
     * @brief
     *   Handler for peer's messages, exchanged during pairing handshake.
     *
     * @param packetHeader Message header for the received message
     * @param peerAddress  Source of the message
     * @param msg          Message sent by the peer
     * @return CHIP_ERROR The result of message processing
     */
    virtual CHIP_ERROR HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                         System::PacketBufferHandle msg);

    /**
     * @brief
     *  Return the associated secure session peer NodeId
     *
     * @return NodeId The associated peer NodeId
     */
    NodeId GetPeerNodeId() const { return mConnectionState.GetPeerNodeId(); }

    /**
     * @brief
     *  Return the associated peer key id
     *
     * @return uint16_t The associated peer key id
     */
    uint16_t GetPeerKeyId() { return mConnectionState.GetPeerKeyID(); }

    /**
     * @brief
     *  Return the associated local key id
     *
     * @return uint16_t The assocated local key id
     */
    uint16_t GetLocalKeyId() { return mConnectionState.GetLocalKeyID(); }

    Transport::PeerConnectionState & PeerConnection() { return mConnectionState; }

private:
    enum SigmaErrorType : uint8_t
    {
        kNoSharedTrustRoots   = 0x01,
        kInvalidSignature     = 0x04,
        kInvalidResumptionTag = 0x05,
        kUnsupportedVersion   = 0x06,
        kUnexpected           = 0xff,
    };

    CHIP_ERROR SendSigmaR1();
    CHIP_ERROR HandleSigmaR1_and_SendSigmaR2(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleSigmaR2_and_SendSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg);
    CHIP_ERROR HandleSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR SendSigmaR1Resume();
    CHIP_ERROR HandleSigmaR1Resume_and_SendSigmaR2Resume(const PacketHeader & header, const System::PacketBufferHandle & msg);

    void SendErrorMsg(SigmaErrorType errorCode);
    void HandleErrorMsg(const PacketHeader & header, const System::PacketBufferHandle & msg);

    CHIP_ERROR AttachHeaderAndSend(Protocols::SecureChannel::MsgType msgType, System::PacketBufferHandle msgBuf);

    void Clear();

    SessionEstablishmentDelegate * mDelegate = nullptr;

    Protocols::SecureChannel::MsgType mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    struct SigmaErrorMsg
    {
        SigmaErrorType error;
    };

protected:
    NodeId mLocalNodeId = kUndefinedNodeId;

    bool mPairingComplete = false;

    Transport::PeerConnectionState mConnectionState;
};

} // namespace chip
