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

#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ClusterEnums.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Pool.h>
#include <crypto/CHIPCryptoPAL.h>
#include <credentials/FabricTable.h>

namespace chip {
namespace Credentials {

using SecurityPolicy = app::Clusters::GroupKeyManagement::GroupKeySecurityPolicyEnum;
static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

// typedef struct GroupOperationalCredentials
// {
//     /// Validity start time in microseconds since 2000-01-01T00:00:00 UTC ("the Epoch")
//     uint64_t start_time;
//     /// Session Id
//     uint16_t hash;
//     /// Operational group key
//     uint8_t encryption_key[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
//     /// Privacy key
//     uint8_t privacy_key[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
// } GroupOperationalCredentials;

// An EpochKey is a single key usable to determine an operational group key
struct EpochKey
{
    static constexpr size_t kLengthBytes = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;
    // Validity start time in microseconds since 2000-01-01T00:00:00 UTC ("the Epoch")
    uint64_t start_time;
    // Actual key bits. Depending on context, it may be a raw epoch key (as seen within `SetKeySet` calls)
    // or it may be the derived operational group key (as seen in any other usage).
    uint8_t key[kLengthBytes];

    void Clear()
    {
        start_time = 0;
        Crypto::ClearSecretData(&key[0], sizeof(key));
    }
};

// A operational group key set, usable by many GroupState mappings
struct KeySet
{
    static constexpr size_t kEpochKeysMax = 3;

    KeySet() = default;
    KeySet(uint16_t id, SecurityPolicy policy_id, uint8_t num_keys) : keyset_id(id), policy(policy_id), num_keys_used(num_keys)
    {}

    // Logical id provided by the Administrator that configured the entry
    uint16_t keyset_id = 0;
    // Security policy to use for groups that use this keyset
    SecurityPolicy policy = SecurityPolicy::kCacheAndSync;
    // Number of keys present
    uint8_t num_keys_used = 0;
    // The actual keys for the group key set
    EpochKey epoch_keys[kEpochKeysMax];

    bool operator==(const KeySet & other) const
    {
        VerifyOrReturnError(this->policy == other.policy && this->num_keys_used == other.num_keys_used, false);
        return !memcmp(this->epoch_keys, other.epoch_keys, this->num_keys_used * sizeof(EpochKey));
    }

    void ClearKeys()
    {
        for (size_t key_idx = 0; key_idx < kEpochKeysMax; ++key_idx)
        {
            epoch_keys[key_idx].Clear();
        }
    }
};

struct GroupSession
{
    GroupSession()   = default;
    FabricIndex fabric_index;
    GroupId group_id = kUndefinedGroupId;
    SecurityPolicy security_policy;
    Crypto::SymmetricKeyContext * keyContext = nullptr;
};

// using KeyContext           = chip::Crypto::SymmetricKeyContext;
using KeySetIterator       = CommonIterator<KeySet>;
using KeyContextIterator   = CommonIterator<chip::Crypto::SymmetricKeyContext*&>;
using GroupSessionIterator = CommonIterator<GroupSession>;
static constexpr KeysetId kIdentityProtectionKeySetId = 0;

class KeyManager
{
public:
    // KeyManager(Crypto::SessionKeystore * store): mKeystore(store) {}
    virtual ~KeyManager() = default;
    virtual bool IsInitialized() = 0;
    virtual void Finish() = 0;

    virtual CHIP_ERROR SetKeySet(FabricIndex fabric_index, const ByteSpan & compressed_fabric_id, const KeySet & keys) = 0;
    virtual CHIP_ERROR GetKeySet(FabricIndex fabric_index, KeysetId keyset_id, KeySet & keys)                          = 0;
    virtual CHIP_ERROR RemoveKeySet(FabricIndex fabric_index, KeysetId keyset_id)                                      = 0;

    /**
     *  Creates an iterator that may be used to obtain the list of key sets associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the key sets table during the iteration is currently not supported, and may yield unexpected behaviour.
     *
     *  @retval An instance of KeySetIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual KeySetIterator * IterateKeySets(FabricIndex fabric_index) = 0;

    /**
     * @brief Obtain the actual operational Identity Protection Key (IPK) keyset for a given
     *        fabric. These keys are used by the CASE protocol, and do not participate in
     *        any direct traffic encryption. Since the identity protection operational keyset
     *        is used in multiple key derivations and procedures, it cannot be hidden behind a
     *        SymmetricKeyContext, and must be obtainable by value.
     *
     * @param fabric_index - Fabric index for which to get the IPK operational keyset
     * @param out_keyset - Reference to a KeySet where the IPK keys will be stored on success
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if the IPK keyset is somehow unavailable
     *         or another CHIP_ERROR value if an internal storage error occurs.
     */
    virtual CHIP_ERROR GetIpkKeySet(FabricIndex fabric_index, KeySet & out_keyset) = 0;

    /**
    * @brief Utility Set the IPK Epoch key on a KeyManager assuming a single IPK
    *
    * This utility replaces having to call `KeyManager::SetKeySet` for the simple situation of a
    * single IPK for a fabric, if a single epoch key is used. Start time will be set to 0 ("was always valid")
    *
    * @param fabric_index - fabric index within the KeyManager for which to set the IPK
    * @param ipk_epoch_span - Span containing the IPK epoch key
    * @param compressed_fabric_id - Compressed fabric ID associated with the fabric, for key derivation
    * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT on any bad argument, other CHIP_ERROR values
    *         from implementation on other errors
    */
    inline CHIP_ERROR SetSingleIpkEpochKey(FabricIndex fabric_index, const ByteSpan & ipk_epoch_span, const ByteSpan & compressed_fabric_id)
    {
        KeySet ipkKeySet(kIdentityProtectionKeySetId, SecurityPolicy::kTrustFirst, 1);

        VerifyOrReturnError(ipk_epoch_span.size() == sizeof(ipkKeySet.epoch_keys[0].key), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(compressed_fabric_id.size() == sizeof(uint64_t), CHIP_ERROR_INVALID_ARGUMENT);

        ipkKeySet.epoch_keys[0].start_time = 0;
        memcpy(&ipkKeySet.epoch_keys[0].key, ipk_epoch_span.data(), ipk_epoch_span.size());

        // Set a single IPK, validate key derivation follows spec
        return SetKeySet(fabric_index, compressed_fabric_id, ipkKeySet);
    }

    virtual Crypto::SymmetricKeyContext * CreateKeyContext(FabricIndex fabric_index, KeysetId keyset_id) = 0;
    virtual CHIP_ERROR GetGroupSession(FabricIndex fabric_index, KeysetId keyset_id, uint16_t hash, GroupSession &session) = 0;

    // Fabrics
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric_index) = 0;
};

/**
 * Instance getter for the global KeyManager.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Group Data Provider
 */
KeyManager * GetKeyManager();

/**
 * Instance setter for the global KeyManager.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * The `provider` can be set to nullptr if the owner is done with it fully.
 *
 * @param[in] provider pointer to the Group Data Provider global isntance to use
 */
void SetKeyManager(KeyManager * provider);

} // namespace Credentials
} // namespace chip
