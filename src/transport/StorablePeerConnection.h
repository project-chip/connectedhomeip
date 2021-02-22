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

#include <core/CHIPPersistentStorageDelegate.h>
#include <transport/PASESession.h>

namespace chip {

// KVS store is sensitive to length of key strings, based on the underlying
// platform. Keeping them short.
constexpr char kStorablePeerConnectionKeyPrefix[] = "CHIPCnxn";
constexpr char kStorablePeerConnectionCountKey[]  = "CHIPNxtCnxn";

class DLL_EXPORT StorablePeerConnection
{
public:
    StorablePeerConnection() {}

    StorablePeerConnection(PASESession & session, Transport::AdminId admin);

    virtual ~StorablePeerConnection() {}

    CHIP_ERROR StoreIntoKVS(PersistentStorageDelegate & kvs);

    CHIP_ERROR FetchFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId);

    static CHIP_ERROR DeleteFromKVS(PersistentStorageDelegate & kvs, uint16_t keyId);

    void GetPASESession(PASESession * session) { session->FromSerializable(mSession.mOpCreds); }

    Transport::AdminId GetAdminId() { return mSession.mAdmin; }

private:
    static constexpr size_t KeySize();

    static CHIP_ERROR GenerateKey(uint16_t id, char * key, size_t len);

    struct StorableSession
    {
        PASESessionSerializable mOpCreds;
        Transport::AdminId mAdmin; /* This field is serialized in LittleEndian byte order */
    };

    StorableSession mSession;
    uint16_t mKeyId;
};

} // namespace chip
