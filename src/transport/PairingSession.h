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
#include <transport/MessageCounter.h>
#include <transport/SecureSession.h>

namespace chip {

class DLL_EXPORT PairingSession
{
public:
    PairingSession() {}
    virtual ~PairingSession() {}

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param session     Referene to the secure session that will be
     *                    initialized once pairing is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role) = 0;

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

    /**
     * @brief
     *   Get the value of peer session counter which is synced during session establishment
     */
    virtual uint32_t GetPeerCounter()
    {
        // TODO(#6652): This is a stub implementation, should be replaced by the real one when CASE and PASE is completed
        return LocalSessionMessageCounter::kInitialValue;
    }
};

} // namespace chip
