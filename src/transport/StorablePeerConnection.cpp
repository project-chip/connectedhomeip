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

#include <core/CHIPEncoding.h>
#include <support/ReturnMacros.h>
#include <support/SafeInt.h>
#include <transport/StorablePeerConnection.h>

namespace chip {

StorablePeerConnection::StorablePeerConnection(PASESession & session, Transport::AdminId admin)
{
    session.ToSerializable(mSession.mOpCreds);
    mSession.mAdmin = Encoding::LittleEndian::HostSwap16(admin);
    mKeyId          = session.GetLocalKeyId();
}

CHIP_ERROR StorablePeerConnection::StoreIntoKVS(PersistentStorageDelegate & kvs)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mKeyId, key, sizeof(key)));

    return kvs.SetKeyValue(key, &mSession, sizeof(mSession));
}

CHIP_ERROR StorablePeerConnection::FetchFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(keyId, key, sizeof(key)));

    uint16_t size = sizeof(mSession);
    return kvs.GetKeyValue(key, &mSession, size);
}

CHIP_ERROR StorablePeerConnection::DeleteFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(keyId, key, sizeof(key)));

    kvs.DeleteKeyValue(key);
    return CHIP_NO_ERROR;
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
