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

#include "StorablePeerConnection.h"

#include <core/CHIPPersistentStorageDelegate.h>
#include <transport/PairingSession.h>
#include <transport/PeerConnectionState.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

namespace {
// KVS store is sensitive to length of key strings, based on the underlying
// platform. Keeping them short.
constexpr char kStorablePeerConnectionKeyPrefix[] = "CHIPConn";
constexpr char kStorablePeerConnectionCountKey[]  = "CHIPConnCnt";

class FetchedSession : public PairingSession
{
public:
    FetchedSession(const SecureSession::Serializable & secureSession, uint16_t localKeyId, uint16_t peerKeyId) :
        mSecureSessionSerializable(secureSession), mLocalKeyId(localKeyId), mPeerKeyId(peerKeyId)
    {}

    CHIP_ERROR DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role) override
    {
        return session.InitFromSerializable(mSecureSessionSerializable, role);
    }

    uint16_t GetPeerKeyId() override { return mPeerKeyId; };
    uint16_t GetLocalKeyId() override { return mLocalKeyId; };

private:
    const SecureSession::Serializable & mSecureSessionSerializable;
    uint16_t mLocalKeyId;
    uint16_t mPeerKeyId;
};

} // namespace

CHIP_ERROR StorablePeerConnection::Init(const Transport::PeerConnectionState & connState)
{
    ReturnErrorCodeIf(!connState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    connState.GetSecureSession().ToSerializable(mSerializable.mSecureSession);
    mSerializable.mPeerKeyId = Encoding::LittleEndian::HostSwap16(connState.GetPeerKeyID());
    mSerializable.mAdmin     = Encoding::LittleEndian::HostSwap16(connState.GetAdminId());
    mKeyId                   = connState.GetLocalKeyID();
    return CHIP_NO_ERROR;
}

CHIP_ERROR StorablePeerConnection::StoreIntoKVS(PersistentStorageDelegate & kvs)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mKeyId, key, sizeof(key)));

    return kvs.SyncSetKeyValue(key, &mSerializable, sizeof(mSerializable));
}

CHIP_ERROR StorablePeerConnection::FetchFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(keyId, key, sizeof(key)));

    mKeyId        = keyId;
    uint16_t size = sizeof(mSerializable);
    return kvs.SyncGetKeyValue(key, &mSerializable, size);
}

CHIP_ERROR StorablePeerConnection::DeleteFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(keyId, key, sizeof(key)));

    return kvs.SyncDeleteKeyValue(key);
}

CHIP_ERROR StorablePeerConnection::StoreCountIntoKVS(PersistentStorageDelegate & kvs, uint16_t count)
{
    count = Encoding::LittleEndian::HostSwap16(count);
    return kvs.SyncSetKeyValue(kStorablePeerConnectionCountKey, &count, sizeof(count));
}

CHIP_ERROR StorablePeerConnection::FetchCountFromKVS(PersistentStorageDelegate & kvs, uint16_t & count)
{
    uint16_t size = sizeof(count);
    ReturnErrorOnFailure(kvs.SyncGetKeyValue(kStorablePeerConnectionCountKey, &count, size));

    count = Encoding::LittleEndian::HostSwap16(count);
    return CHIP_NO_ERROR;
}

CHIP_ERROR StorablePeerConnection::DeleteCountFromKVS(PersistentStorageDelegate & kvs)
{
    return kvs.SyncDeleteKeyValue(kStorablePeerConnectionCountKey);
}

CHIP_ERROR StorablePeerConnection::AddNewPairing(SecureSessionMgr & mgr, SecureSession::SessionRole role)
{
    Transport::AdminId adminId = Encoding::LittleEndian::HostSwap16(mSerializable.mAdmin);
    uint16_t peerKeyId         = Encoding::LittleEndian::HostSwap16(mSerializable.mPeerKeyId);
    FetchedSession session{ mSerializable.mSecureSession, mKeyId, peerKeyId };

    return mgr.NewPairing(Optional<Transport::PeerAddress>{}, NodeId{}, &session, role, adminId);
}

constexpr size_t StorablePeerConnection::KeySize()
{
    return sizeof(kStorablePeerConnectionKeyPrefix) + 2 * sizeof(uint16_t);
}

CHIP_ERROR StorablePeerConnection::GenerateKey(uint16_t id, char * key, size_t len)
{
    VerifyOrReturnError(len >= KeySize(), CHIP_ERROR_INVALID_ARGUMENT);
    int keySize = snprintf(key, len, "%s%x", kStorablePeerConnectionKeyPrefix, id);
    VerifyOrReturnError(keySize > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(len > (size_t) keySize, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

} // namespace chip
