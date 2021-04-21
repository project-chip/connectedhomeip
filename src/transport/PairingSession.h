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
 *      This file defines a common interface to access various types of secure
 *      pairing sessions (e.g. PASE, CASE)
 *
 */

#pragma once

#include <core/CHIPError.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <transport/PeerConnectionState.h>
#include <transport/SecureSession.h>
#include <transport/SessionEstablishmentDelegate.h>

namespace chip {

class DLL_EXPORT PairingSession : public Messaging::ExchangeDelegateBase
{
public:
    PairingSession() {}
    virtual ~PairingSession() {}

    enum SecureSessionType : uint8_t
    {
        kPASESession = 0x00,
        kCASESession = 0x01,
        kUnexpected  = 0xff
    };

    enum SessionParameter : uint8_t
    {
        kSetupPinCode             = 0x00,
        kPASEVerifier             = 0x01,
        kCertificateCredentialSet = 0x02,
        kParameterUnexpected      = 0xff,
    };

    virtual CHIP_ERROR WaitForPairing(const void * mySessionParameters, SessionParameter sessionParameter, uint16_t myKeyId,
                                      SessionEstablishmentDelegate * delegate)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    virtual CHIP_ERROR Pair(const Transport::PeerAddress peerAddress, const void * arg, uint16_t myKeyId,
                            Messaging::ExchangeContext * exchangeCtxt, SessionEstablishmentDelegate * delegate)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param info        Information string used for key derivation
     * @param info_len    Length of info string
     * @param session     Referene to the secure session that will be
     *                    initialized once pairing is complete
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session) = 0;

    /**
     * @brief
     *  Return the associated peer key id
     *
     * @return uint16_t The associated peer key id
     */
    virtual uint16_t GetPeerKeyId() = 0;

    /**
     * @brief
     *  Return the associated local key id
     *
     * @return uint16_t The associated local key id
     */
    virtual uint16_t GetLocalKeyId() = 0;

    virtual const char * GetI2RSessionInfo() const = 0;

    virtual const char * GetR2ISessionInfo() const = 0;

    virtual Transport::PeerConnectionState & PeerConnection() = 0;

    virtual CHIP_ERROR ToSerializable(void * output) { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual CHIP_ERROR FromSerializable(const void * output) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual void Clear() {}

    virtual SecureSessionType GetSecureSessionType() { return SecureSessionType::kUnexpected; }

    virtual SessionEstablishmentExchangeDispatch & MessageDispatch() = 0;

    //// ExchangeDelegate Implementation ////
    virtual void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                   const PayloadHeader & payloadHeader, System::PacketBufferHandle payload) = 0;
    virtual void OnResponseTimeout(Messaging::ExchangeContext * ec)                                         = 0;
    virtual Messaging::ExchangeMessageDispatch * GetMessageDispatch(Messaging::ReliableMessageMgr * rmMgr,
                                                                    SecureSessionMgr * sessionMgr)          = 0;
};

} // namespace chip
