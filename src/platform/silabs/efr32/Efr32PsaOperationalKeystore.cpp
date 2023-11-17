/*
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "Efr32PsaOperationalKeystore.h"

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

#include "Efr32OpaqueKeypair.h"
#include <platform/silabs/SilabsConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static_assert((sizeof(FabricIndex) == 1), "Implementation is not prepared for large fabric indices");
static_assert(SL_MATTER_MAX_STORED_OP_KEYS <= (kEFR32OpaqueKeyIdPersistentMax - kEFR32OpaqueKeyIdPersistentMin),
              "Not enough opaque keys available to cover all requested operational keys");
static_assert((CHIP_CONFIG_MAX_FABRICS + 1) <= SL_MATTER_MAX_STORED_OP_KEYS,
              "Not enough operational keys requested to cover all potential fabrics (+1 staging for fabric update)");
static_assert(SL_MATTER_MAX_STORED_OP_KEYS >= 1, "Minimum supported amount of operational keys is 1");

using namespace chip::Crypto;

using chip::Platform::MemoryCalloc;
using chip::Platform::MemoryFree;

Efr32PsaOperationalKeystore::~Efr32PsaOperationalKeystore()
{
    Deinit();
}

CHIP_ERROR Efr32PsaOperationalKeystore::Init()
{
    // Detect existing keymap size
    CHIP_ERROR error   = CHIP_NO_ERROR;
    size_t wantedLen   = SL_MATTER_MAX_STORED_OP_KEYS * sizeof(FabricIndex);
    size_t existingLen = 0;
    bool update_cache  = false;

    if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_OpKeyMap, existingLen))
    {
        // There's a pre-existing key map on disk. Size the map to read it fully.
        size_t outLen = 0;

        if (existingLen > (kEFR32OpaqueKeyIdPersistentMax - kEFR32OpaqueKeyIdPersistentMin) * sizeof(FabricIndex))
        {
            return CHIP_ERROR_INTERNAL;
        }

        // Upsize the map if the config was changed
        if (existingLen < wantedLen)
        {
            existingLen = wantedLen;
        }

        mKeyMap = (FabricIndex *) MemoryCalloc(1, existingLen);
        VerifyOrExit(mKeyMap, error = CHIP_ERROR_NO_MEMORY);

        // Read the existing key map
        error = SilabsConfig::ReadConfigValueBin(SilabsConfig::kConfigKey_OpKeyMap, (uint8_t *) mKeyMap, existingLen, outLen);
        SuccessOrExit(error);

        // If upsizing, extend the map with undefined indices
        for (size_t i = (outLen / sizeof(FabricIndex)); i < (existingLen / sizeof(FabricIndex)); i++)
        {
            mKeyMap[i] = kUndefinedFabricIndex;
        }

        // If the config has changed, check whether it can be downsized fully or partially
        if (existingLen > wantedLen)
        {
            size_t highest_found_index = 0;
            for (size_t i = (wantedLen / sizeof(FabricIndex)); i < (existingLen / sizeof(FabricIndex)); i++)
            {
                if (mKeyMap[i] != kUndefinedFabricIndex)
                {
                    highest_found_index = i;
                }
            }

            // set size to the smallest that will fit the upper opaque key ID in use
            if (highest_found_index > 0)
            {
                existingLen  = (highest_found_index + 1) * sizeof(FabricIndex);
                update_cache = true;
            }
        }

        // Set the key map size
        mKeyMapSize = existingLen;
    }
    else
    {
        // No key map on disk. Create and initialize a new one.
        mKeyMap = (FabricIndex *) MemoryCalloc(1, wantedLen);
        VerifyOrExit(mKeyMap, error = CHIP_ERROR_NO_MEMORY);

        for (size_t i = 0; i < (wantedLen / sizeof(FabricIndex)); i++)
        {
            mKeyMap[i] = kUndefinedFabricIndex;
        }

        mKeyMapSize = wantedLen;

        update_cache = true;
    }

    // Write-out keymap if needed
    if (update_cache)
    {
        error = SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_OpKeyMap, mKeyMap, mKeyMapSize);
        SuccessOrExit(error);
    }

    // Initialize cache key
    mCachedKey = Platform::New<EFR32OpaqueP256Keypair>();
    VerifyOrExit(mCachedKey, error = CHIP_ERROR_NO_MEMORY);

exit:
    if (error != CHIP_NO_ERROR)
    {
        Deinit();
        return error;
    }

    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

EFR32OpaqueKeyId Efr32PsaOperationalKeystore::FindKeyIdForFabric(FabricIndex fabricIndex) const
{
    // Search the map linearly to find a matching index slot
    for (size_t i = 0; i < (mKeyMapSize / sizeof(FabricIndex)); i++)
    {
        if (mKeyMap[i] == fabricIndex)
        {
            // Found a match
            return i + kEFR32OpaqueKeyIdPersistentMin;
        }
    }

    return kEFR32OpaqueKeyIdUnknown;
}

bool Efr32PsaOperationalKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(mIsInitialized, false);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && (mPendingKeypair != nullptr))
    {
        return true;
    }

    // Check whether we have a match in the map
    if (FindKeyIdForFabric(fabricIndex) != kEFR32OpaqueKeyIdUnknown)
    {
        return true;
    }

    return false;
}

CHIP_ERROR Efr32PsaOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                              MutableByteSpan & outCertificateSigningRequest)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Generate new key
    EFR32OpaqueKeyId id = kEFR32OpaqueKeyIdUnknown;

    if (mPendingFabricIndex != kUndefinedFabricIndex)
    {
        // If we already have a pending key, delete it and put a new one in its place
        id = mPendingKeypair->GetKeyId();
        if (id == kEFR32OpaqueKeyIdUnknown)
        {
            ResetPendingKey();
        }
        else
        {
            mPendingKeypair->DestroyKey();
            if (id == kEFR32OpaqueKeyIdVolatile)
            {
                id = kEFR32OpaqueKeyIdUnknown;
            }
        }
    }

    if (id == kEFR32OpaqueKeyIdUnknown)
    {
        // Try to find an available opaque ID in the map
        id = FindKeyIdForFabric(kUndefinedFabricIndex);

        if (!mPendingKeypair)
        {
            mPendingKeypair = Platform::New<EFR32OpaqueP256Keypair>();
        }
    }

    if (id == kEFR32OpaqueKeyIdUnknown)
    {
        // Could not find a free spot in the map
        return CHIP_ERROR_NO_MEMORY;
    }

    // Create new key on the old or found key ID
    error = mPendingKeypair->Create(id, EFR32OpaqueKeyUsages::ECDSA_P256_SHA256);
    if (error != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return error;
    }

    // Set CSR and state
    size_t csrLength = outCertificateSigningRequest.size();
    error            = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
    if (error != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return error;
    }

    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Efr32PsaOperationalKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                   const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Efr32PsaOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Add key association to key map
    EFR32OpaqueKeyId id = mPendingKeypair->GetKeyId();

    if (id == kEFR32OpaqueKeyIdUnknown || id == kEFR32OpaqueKeyIdVolatile)
    {
        ResetPendingKey();
        return CHIP_ERROR_INTERNAL;
    }

    // Guard against array out-of-bounds (should not happen with correctly initialised keys)
    size_t keymap_index = id - kEFR32OpaqueKeyIdPersistentMin;
    if (keymap_index >= (mKeyMapSize / sizeof(FabricIndex)))
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (mKeyMap[keymap_index] != kUndefinedFabricIndex)
    {
        ResetPendingKey();
        return CHIP_ERROR_INTERNAL;
    }

    mKeyMap[keymap_index] = fabricIndex;

    // Persist key map
    CHIP_ERROR error = SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_OpKeyMap, mKeyMap, mKeyMapSize);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }

    // There's a good chance we'll need the key again soon
    mCachedKey->Load(id);

    ResetPendingKey(true /* keepKeyPairInStorage */);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Efr32PsaOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Remove pending keypair if we have it and the fabric ID matches
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    EFR32OpaqueKeyId id = FindKeyIdForFabric(fabricIndex);
    if (id == kEFR32OpaqueKeyIdUnknown)
    {
        // Fabric is not in the map, so assume it's gone already
        return CHIP_NO_ERROR;
    }

    // Guard against array out-of-bounds (should not happen with correctly initialised keys)
    size_t keymap_index = id - kEFR32OpaqueKeyIdPersistentMin;
    if (keymap_index >= (mKeyMapSize / sizeof(FabricIndex)))
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Reset the key mapping since we'll be deleting this key
    mKeyMap[keymap_index] = kUndefinedFabricIndex;

    // Persist key map
    CHIP_ERROR error = SilabsConfig::WriteConfigValueBin(SilabsConfig::kConfigKey_OpKeyMap, mKeyMap, mKeyMapSize);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }

    // Check if key is cached
    EFR32OpaqueKeyId cachedId = mCachedKey->GetKeyId();

    if (id == cachedId)
    {
        // Delete from persistent storage and unload
        mCachedKey->DestroyKey();
        return CHIP_NO_ERROR;
    }

    // Load it for purposes of deletion
    error = mCachedKey->Load(id);
    if (error != CHIP_NO_ERROR && error != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        return CHIP_ERROR_INTERNAL;
    }

    mCachedKey->DestroyKey();

    return CHIP_NO_ERROR;
}

void Efr32PsaOperationalKeystore::RevertPendingKeypair()
{
    if (mIsInitialized)
    {
        // Just delete the pending key from storage
        ResetPendingKey();
    }
}

CHIP_ERROR Efr32PsaOperationalKeystore::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                          Crypto::P256ECDSASignature & outSignature) const
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Check to see whether the key is an activated pending key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    // Figure out which key ID we're looking for
    EFR32OpaqueKeyId id = FindKeyIdForFabric(fabricIndex);

    if (id == kEFR32OpaqueKeyIdUnknown)
    {
        // Fabric is not in the map, but the caller thinks it's there?
        return CHIP_ERROR_INTERNAL;
    }

    // Check whether we have the key in cache
    EFR32OpaqueKeyId cachedId = mCachedKey->GetKeyId();

    if (id == cachedId)
    {
        return mCachedKey->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    // If not, we need to recreate from the backend
    CHIP_ERROR error = mCachedKey->Load(id);
    if (error != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Sign with retrieved key
    error = mCachedKey->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    if (error != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

Crypto::P256Keypair * Efr32PsaOperationalKeystore::AllocateEphemeralKeypairForCASE()
{
    EFR32OpaqueP256Keypair * new_key = Platform::New<EFR32OpaqueP256Keypair>();

    if (new_key != nullptr)
    {
        new_key->Create(kEFR32OpaqueKeyIdVolatile, EFR32OpaqueKeyUsages::ECDH_P256);
    }

    return new_key;
}

void Efr32PsaOperationalKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<EFR32OpaqueP256Keypair>((EFR32OpaqueP256Keypair *) keypair);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
