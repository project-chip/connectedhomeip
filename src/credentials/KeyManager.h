/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <optional>
#include <stdint.h>
#include <sys/types.h>

#include <app/util/basic-types.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ClusterEnums.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace Credentials {

class KeyManager {

    struct KeyContext : chip::Crypto::SymmetricKeyContext
    {
        KeyContext(DataProvider & provider): mProvider(provider) {}
        KeyContext(DataProvider & provider, const GroupKey & key): mProvider(provider) { Set(key); }

        void Set(const GroupKey & key);
        uint16_t GetKeyHash() override;
        CHIP_ERROR MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce, MutableByteSpan & mic,
                                  MutableByteSpan & ciphertext) const override;
        CHIP_ERROR MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce, const ByteSpan & mic,
                                  MutableByteSpan & plaintext) const override;
        CHIP_ERROR PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        CHIP_ERROR PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        void Release() override;

    private:
        DataProvider & mProvider;
        uint16_t mHash;
        Crypto::Aes128KeyHandle mEncryptionKey;
        Crypto::Aes128KeyHandle mPrivacyKey;
    };

    
    struct KeyIterator : CommonIterator<KeyContext*&>
    {
        KeyIterator(DataProvider & provider, FabricTable *fabrics, GroupId group_id, uint16_t session_id);
        size_t Count() override;
        bool Next(KeyContext *&output) override;
        void Release() override;

    private:
        DataProvider & mProvider;
        FabricTable *mFabrics = nullptr;
        GroupId mGroupId;
        KeyContext mContext;
        uint16_t mSessionId = 0;
        uint8_t mFabricIndex = 0;
        size_t mKeyIndex = 0;
    };

    /**
     *  Creates an iterator that may be used to obtain the list of key sets associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the key sets table during the iteration is currently not supported, and may yield unexpected behaviour.
     *
     *  @retval An instance of KeySetIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual KeySetIterator * IterateKeySets(FabricIndex fabric_index);

    virtual Crypto::SymmetricKeyContext * GetKeyContext(FabricIndex fabric_index, GroupId group_id);

privateL:
    ObjectPool<KeySetIteratorImpl, kIteratorsMax> mKeySetIterators;
    ObjectPool<GroupSessionIteratorImpl, kIteratorsMax> mGroupSessionsIterator;
    ObjectPool<GroupKeyContext, kIteratorsMax> mGroupKeyContexPool;
};

} // namespace Credentials
} // namespace chip
