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

#include <app/server/StorablePeerConnection.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/SafeInt.h>

namespace chip {

StorablePeerConnection::StorablePeerConnection(PASESession & session, FabricIndex fabric)
{
    session.ToSerializable(mSession.mOpCreds);
    mSession.mFabric = fabric;
    mKeyId           = session.GetLocalKeyId();
}

CHIP_ERROR StorablePeerConnection::StoreIntoKVS(PersistentStorageDelegate & kvs)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(mKeyId, key, sizeof(key)));

    return kvs.SyncSetKeyValue(key, &mSession, sizeof(mSession));
}

CHIP_ERROR StorablePeerConnection::FetchFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(keyId, key, sizeof(key)));

    uint16_t size = sizeof(mSession);
    return kvs.SyncGetKeyValue(key, &mSession, size);
}

CHIP_ERROR StorablePeerConnection::DeleteFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId)
{
    char key[KeySize()];
    ReturnErrorOnFailure(GenerateKey(keyId, key, sizeof(key)));

    return kvs.SyncDeleteKeyValue(key);
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
