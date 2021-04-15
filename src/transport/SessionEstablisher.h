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
 *      This file defines the CHIP SessionEstablisher object that provides
 *      APIs for constructing CASE/PASE messages and establishing encryption
 *      keys.
 *
 */

#pragma once

#include <transport/PeerConnectionState.h>
#include <transport/SecureSession.h>
#include <transport/SessionEstablishmentDelegate.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

using namespace Crypto;

class SessionEstablisher
{
public:
    virtual ~SessionEstablisher(){};

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
                                      SessionEstablishmentDelegate * delegate) = 0;
    virtual CHIP_ERROR Pair(const Transport::PeerAddress peerAddress, const void * arg, uint16_t myKeyId,
                            SessionEstablishmentDelegate * delegate)           = 0;

    virtual Transport::PeerConnectionState & PeerConnection()            = 0;
    virtual uint16_t GetPeerKeyId()                                      = 0;
    virtual uint16_t GetLocalKeyId()                                     = 0;
    virtual CHIP_ERROR HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                         System::PacketBufferHandle msg) = 0;

    virtual CHIP_ERROR DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session) = 0;

    virtual CHIP_ERROR ToSerializable(void * output)         = 0;
    virtual CHIP_ERROR FromSerializable(const void * output) = 0;

    virtual SecureSessionType GetSecureSessionType() = 0;
};

} // namespace chip
