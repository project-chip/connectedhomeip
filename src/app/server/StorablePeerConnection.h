/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <core/CHIPEncoding.h>
#include <transport/AdminPairingTable.h>
#include <transport/SecureSession.h>

namespace chip {

class PersistentStorageDelegate;
class SecureSessionMgr;

namespace Transport {
class PeerConnectionState;
} // namespace Transport

class DLL_EXPORT StorablePeerConnection
{
public:
    StorablePeerConnection() = default;

    CHIP_ERROR Init(const Transport::PeerConnectionState & connState);

    CHIP_ERROR StoreIntoKVS(PersistentStorageDelegate & kvs);
    CHIP_ERROR FetchFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId);
    static CHIP_ERROR DeleteFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId);

    static CHIP_ERROR StoreCountIntoKVS(PersistentStorageDelegate & kvs, uint16_t count);
    static CHIP_ERROR FetchCountFromKVS(PersistentStorageDelegate & kvs, uint16_t & count);
    static CHIP_ERROR DeleteCountFromKVS(PersistentStorageDelegate & kvs);

    CHIP_ERROR AddNewPairing(SecureSessionMgr & mgr, SecureSession::SessionRole role);

private:
    static constexpr size_t KeySize();
    static CHIP_ERROR GenerateKey(uint16_t id, char * key, size_t len);

    struct Serializable
    {
        SecureSession::Serializable mSecureSession;
        uint16_t mPeerKeyId;       // Serialized in LittleEndian byte order
        Transport::AdminId mAdmin; // Serialized in LittleEndian byte order
    };

    Serializable mSerializable;
    uint16_t mKeyId;
};

} // namespace chip
