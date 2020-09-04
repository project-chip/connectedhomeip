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
 *      This file implements interfaces for deriving and retrieving
 *      CHIP constituent and application group keys.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "CHIPApplicationKeys.h"
#include <core/CHIPCore.h>
#include <core/CHIPKeyIds.h>

namespace chip {
namespace Protocols {
namespace Security {
namespace AppKeys {

/**
 * Get application group master key ID given application group global ID.
 *
 * @param[in]   groupGlobalId    The application group global ID.
 * @param[in]   groupKeyStore    A pointer to the group key store object.
 * @param[out]  groupMasterKeyId The application group master key ID.
 *
 * @retval #CHIP_NO_ERROR       On success.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT
 *                               If pointer to the group key store is not provided.
 * @retval #CHIP_ERROR_UNSUPPORTED_WEAVE_FEATURE
 *                               If FabricState object wasn't initialized with fully
 *                               functional group key store.
 * @retval #CHIP_ERROR_KEY_NOT_FOUND
 *                               If a group key with specified global ID is not found
 *                               in the platform key store.
 * @retval other                 Other platform-specific errors returned by the platform
 *                               key store APIs.
 *
 */
CHIP_ERROR GetAppGroupMasterKeyId(uint32_t groupGlobalId, GroupKeyStoreBase * groupKeyStore, uint32_t & groupMasterKeyId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR LogGroupKeys(GroupKeyStoreBase * groupKeyStore)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/**
 * Initialize local group key store parameters.
 */
void GroupKeyStoreBase::Init(void)
{
    LastUsedEpochKeyId    = ChipKeyId::kNone;
    NextEpochKeyStartTime = UINT32_MAX;
}

/**
 * Returns current key ID.
 * Sets member variables associated with epoch keys to the default values when any change
 * (delete or store) happens to the set of application epoch keys. It is the responsibility
 * of the subclass that implements StoreGroupKey(), DeleteGroupKey(), and
 * DeleteGroupKeysOfAType() functions to call this method.
 */
void GroupKeyStoreBase::OnEpochKeysChange(void)
{
    LastUsedEpochKeyId    = ChipKeyId::kNone;
    NextEpochKeyStartTime = UINT32_MAX;
}

/**
 * Get current platform UTC time in seconds.
 *
 * @param[out]   utcTime             A reference to the time value.
 *
 * @retval #CHIP_NO_ERROR           On success.
 * @retval #CHIP_SYSTEM_ERROR_NOT_SUPPORTED
 *                                   If platform does not support a real time clock.
 * @retval #CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED
 *                                   If the system's real time clock is not synchronized to
 *                                   an accurate time source.
 * @retval other                     Other Weave or platform error codes.
 */
CHIP_ERROR GroupKeyStoreBase::GetCurrentUTCTime(uint32_t & utcTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/**
 * Returns current key ID.
 * Finds current epoch key based on the current system time and the start time parameter
 * of each epoch key. If system doesn't have valid, accurate time then last-used epoch key
 * ID is returned.
 *
 * @param[in]    keyId           The application key ID.
 * @param[out]   curKeyId        The application current key ID.
 *
 * @retval #CHIP_NO_ERROR       On success.
 * @retval #CHIP_ERROR_INVALID_KEY_ID
 *                               If the input key ID had an invalid value.
 * @retval #CHIP_ERROR_KEY_NOT_FOUND
 *                               If epoch keys are not found in the platform key store.
 * @retval other                 Other platform-specific errors returned by the platform
 *                               key store APIs.
 *
 */
CHIP_ERROR GroupKeyStoreBase::GetCurrentAppKeyId(uint32_t keyId, uint32_t & curKeyId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/**
 * Get application group key.
 * This function derives or retrieves application group keys. Key types supported by
 * this function are: fabric secret, root key, epoch key, group master key, and intermediate key.
 *
 * @param[in]    keyId           The group key ID.
 * @param[out]   groupKey        A reference to the group key object.
 *
 * @retval #CHIP_NO_ERROR       On success.
 * @retval #CHIP_ERROR_INVALID_KEY_ID
 *                               If the requested key has invalid key ID.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT
 *                               If the platform key store returns invalid key parameters.
 * @retval other                 Other platform-specific errors returned by the platform
 *                               key store APIs.
 *
 */
CHIP_ERROR GroupKeyStoreBase::GetGroupKey(uint32_t keyId, ChipGroupKey & groupKey)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/**
 * Derive fabric/client root key.
 * Fabric and client root keys are derived from fabric secret, which is retrieved
 * from the platform key store.
 *
 * @param[in]   rootKeyId        The key ID associated with the requested root key.
 * @param[out]  rootKey          A reference to the key material object.
 *
 * @retval #CHIP_NO_ERROR       On success.
 * @retval #CHIP_ERROR_INVALID_KEY_ID
 *                               If the requested key has invalid key ID.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT
 *                               If the platform key store returns invalid key parameters.
 * @retval other                 Other platform-specific errors returned by the platform
 *                               key store APIs.
 *
 */
CHIP_ERROR GroupKeyStoreBase::DeriveFabricOrClientRootKey(uint32_t rootKeyId, ChipGroupKey & rootKey)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/**
 * Derive application intermediate key.
 * The intermediate key is derived from the root key and epoch key material specified
 * in the @a keyId input.
 *
 * @param[in]   keyId            The requested intermediate key ID.
 * @param[out]  intermediateKey  A reference to the key material object.
 *
 * @retval #CHIP_NO_ERROR       On success.
 * @retval #CHIP_ERROR_INVALID_KEY_ID
 *                               If the requested key has invalid key ID.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT
 *                               If the platform key store returns invalid key parameters.
 * @retval other                 Other platform-specific errors returned by the platform
 *                               key store APIs.
 *
 */
CHIP_ERROR GroupKeyStoreBase::DeriveIntermediateKey(uint32_t keyId, ChipGroupKey & intermediateKey)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

/**
 * Derives application key.
 * Three types of application keys are supported: current application key, rotating
 * application key and static application key. When current application key is requested
 * the function finds and uses the current epoch key based on the current system time and
 * the start time parameter of each epoch key.
 *
 * @param[inout] keyId              A reference to the requested key ID. When current application
 *                                  key is requested this field is updated to reflect the new
 *                                  type (rotating application key) and the actual epoch key ID
 *                                  that was used to generate application key.
 * @param[in]    keySalt            A pointer to a buffer with application key salt value.
 * @param[in]    saltLen            The length of the application key salt.
 * @param[in]    keyDiversifier     A pointer to a buffer with application key diversifier value.
 * @param[in]    diversifierLen     The length of the application key diversifier.
 * @param[out]   appKey             A pointer to a buffer where the derived key will be written.
 * @param[in]    keyBufSize         The length of the supplied key buffer.
 * @param[in]    keyLen             The length of the requested key material.
 * @param[out]   appGroupGlobalId   The application group global ID of the associated key.
 *
 * @retval #CHIP_NO_ERROR          On success.
 * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
 *                                  If the provided key buffer size is not sufficient.
 * @retval #CHIP_ERROR_INVALID_KEY_ID
 *                                  If the requested key has invalid key ID.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT
 *                                  If the platform key store returns invalid key parameters
 *                                  or key identifier has invalid value.
 * @retval other                    Other platform-specific errors returned by the platform
 *                                  key store APIs.
 *
 */
CHIP_ERROR GroupKeyStoreBase::DeriveApplicationKey(uint32_t & keyId, const uint8_t * keySalt, uint8_t saltLen,
                                                   const uint8_t * keyDiversifier, uint8_t diversifierLen, uint8_t * appKey,
                                                   uint8_t keyBufSize, uint8_t keyLen, uint32_t & appGroupGlobalId)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace AppKeys
} // namespace Security
} // namespace Protocols
} // namespace chip
