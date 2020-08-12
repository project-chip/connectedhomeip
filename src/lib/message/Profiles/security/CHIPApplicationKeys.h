/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file defines classes and interfaces for deriving and
 *      managing CHIP constituent and application group keys.
 *
 */

#ifndef CHIPAPPLICATIONKEYS_H_
#define CHIPAPPLICATIONKEYS_H_

#include <core/CHIPCore.h>

/**
 *   @namespace chip::Profiles::Security::AppKeys
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the CHIP
 *     application keys library within the CHIP security profile.
 */

namespace chip {
namespace Profiles {
namespace Security {
namespace AppKeys {

/**
 * @brief
 *   Key diversifier used for CHIP fabric root key derivation. This value represents
 *   first 4 bytes of the SHA-1 HASH of "Fabric Root Key" phrase.
 */
extern const uint8_t kChipAppFabricRootKeyDiversifier[4];

/**
 * @brief
 *   Key diversifier used for CHIP client root key derivation. This value represents
 *   first 4 bytes of the SHA-1 HASH of "Client Root Key" phrase.
 */
extern const uint8_t kChipAppClientRootKeyDiversifier[4];

/**
 * @brief
 *   Key diversifier used for CHIP intermediate key derivation. This value represents
 *   first 4 bytes of the SHA-1 HASH of "Intermediate Key" phrase.
 */
extern const uint8_t kChipAppIntermediateKeyDiversifier[4];

/**
 * @brief
 *   CHIP application keys protocol parameter definitions.
 */
enum
{
    // --- Key sizes.
    kChipAppGroupKeySize        = 32,                   /**< CHIP constituent group key size. */
    kChipAppRootKeySize         = kChipAppGroupKeySize, /**< CHIP application root key size. */
    kChipAppEpochKeySize        = kChipAppGroupKeySize, /**< CHIP application epoch key size. */
    kChipAppGroupMasterKeySize  = kChipAppGroupKeySize, /**< CHIP application group master key size. */
    kChipAppIntermediateKeySize = kChipAppGroupKeySize, /**< CHIP application intermediate key size. */
    kChipFabricSecretSize       = 36,                   /**< CHIP fabric secret size. */

    // --- Key diversifiers sizes.
    /** Fabric root key diversifier size. */
    kChipAppFabricRootKeyDiversifierSize = sizeof(kChipAppFabricRootKeyDiversifier),
    /** Client root key diversifier size. */
    kChipAppClientRootKeyDiversifierSize = sizeof(kChipAppClientRootKeyDiversifier),
    /** Intermediate key diversifier size. */
    kChipAppIntermediateKeyDiversifierSize = sizeof(kChipAppIntermediateKeyDiversifier),
};

/**
 *  @class ChipGroupKey
 *
 *  @brief
 *    Contains information about CHIP application group keys.
 *    Examples of keys that can be described by this class are: root key,
 *    epoch key, group master key, intermediate key, and fabric secret.
 *
 */
class ChipGroupKey
{
public:
    enum
    {
        MaxKeySize = kChipFabricSecretSize
    };
    uint32_t KeyId;          /**< The key ID. */
    uint8_t KeyLen;          /**< The key length. */
    uint8_t Key[MaxKeySize]; /**< The secret key material. */
    union
    {
        uint32_t StartTime; /**< The epoch key start time. */
        uint32_t GlobalId;  /**< The application group key global ID. */
    };
};

/**
 *  @class GroupKeyStoreBase
 *
 *  @brief
 *    The definition of the CHIP group key store class. Functions in
 *    this class are called to manage application group keys.
 *
 */
class DLL_EXPORT GroupKeyStoreBase
{
public:
    // Manage application group key material storage.
    virtual CHIP_ERROR RetrieveGroupKey(uint32_t keyId, ChipGroupKey & key)                                                 = 0;
    virtual CHIP_ERROR StoreGroupKey(const ChipGroupKey & key)                                                              = 0;
    virtual CHIP_ERROR DeleteGroupKey(uint32_t keyId)                                                                       = 0;
    virtual CHIP_ERROR DeleteGroupKeysOfAType(uint32_t keyType)                                                             = 0;
    virtual CHIP_ERROR EnumerateGroupKeys(uint32_t keyType, uint32_t * keyIds, uint8_t keyIdsArraySize, uint8_t & keyCount) = 0;
    virtual CHIP_ERROR Clear(void)                                                                                          = 0;

    // Get the current time.
    virtual CHIP_ERROR GetCurrentUTCTime(uint32_t & utcTime);

    // Get current application key Id.
    CHIP_ERROR GetCurrentAppKeyId(uint32_t keyId, uint32_t & curKeyId);

    // Get/Derive group key.
    CHIP_ERROR GetGroupKey(uint32_t keyId, ChipGroupKey & groupKey);

    // Derive application key.
    CHIP_ERROR DeriveApplicationKey(uint32_t & appKeyId, const uint8_t * keySalt, uint8_t saltLen, const uint8_t * keyDiversifier,
                                    uint8_t diversifierLen, uint8_t * appKey, uint8_t keyBufSize, uint8_t keyLen,
                                    uint32_t & appGroupGlobalId);

protected:
    uint32_t LastUsedEpochKeyId;
    uint32_t NextEpochKeyStartTime;

    void Init(void);
    void OnEpochKeysChange(void);

    // Retrieve and Store LastUsedEpochKeyId value.
    virtual CHIP_ERROR RetrieveLastUsedEpochKeyId(void) = 0;
    virtual CHIP_ERROR StoreLastUsedEpochKeyId(void)    = 0;

private:
    // Derive fabric/client root key.
    CHIP_ERROR DeriveFabricOrClientRootKey(uint32_t rootKeyId, ChipGroupKey & rootKey);

    // Derive intermediate key.
    CHIP_ERROR DeriveIntermediateKey(uint32_t keyId, ChipGroupKey & intermediateKey);
};

extern CHIP_ERROR GetAppGroupMasterKeyId(uint32_t groupGlobalId, GroupKeyStoreBase * groupKeyStore, uint32_t & groupMasterKeyId);

extern CHIP_ERROR LogGroupKeys(GroupKeyStoreBase * groupKeyStore);

} // namespace AppKeys
} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPAPPLICATIONKEYS_H_ */
