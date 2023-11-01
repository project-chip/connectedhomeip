/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CodeUtils.h>
#include <stddef.h>

// TODO: SymmetricKeystore is not in sdk code, replace the below when sdk supports SymmetricKeystore
namespace chip {
namespace Crypto {
using SymmetricKeystore = SessionKeystore;
} // namespace Crypto
} // namespace chip

namespace chip {
namespace app {

/**
 * Struct per identity representing persistent storage for ICD client information and key store
 */
struct ICDStorage
{
public:
    FabricIndex mFabricIndex                      = kUndefinedFabricIndex;
    PersistentStorageDelegate * mpClientInfoStore = nullptr;
    Crypto::SymmetricKeystore * mpKeyStore        = nullptr;
    ICDStorage(FabricIndex aFabricIndex, PersistentStorageDelegate * apClientInfoStore, Crypto::SymmetricKeystore * apKeyStore)
    {
        mFabricIndex      = aFabricIndex;
        mpClientInfoStore = apClientInfoStore;
        mpKeyStore        = apKeyStore;
    }
    ~ICDStorage() = default;
    ICDStorage(ICDStorage && aOther)
    {
        mFabricIndex             = aOther.mFabricIndex;
        mpClientInfoStore        = aOther.mpClientInfoStore;
        mpKeyStore               = aOther.mpKeyStore;
        aOther.mpClientInfoStore = nullptr;
        aOther.mpKeyStore        = nullptr;
        aOther.mFabricIndex      = kUndefinedFabricIndex;
    }
    ICDStorage & operator=(const ICDStorage & aOther) = default;
    bool IsValid() { return (mFabricIndex != kUndefinedFabricIndex) && (mpClientInfoStore != nullptr) && (mpKeyStore != nullptr); }
};

} // namespace app
} // namespace chip
