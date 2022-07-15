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

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>

#include "Efr32PsaOperationalKeystore.h"
#include "Efr32OpaqueKeypair.h"

namespace chip {

static_assert((sizeof(FabricIndex) == 1), "Implementation is not prepared for large fabric indices");

using namespace chip::Crypto;

Efr32PsaOperationalKeystore::~Efr32PsaOperationalKeystore()
{
    if (mIsInitialized)
    {
        ResetPendingKey();
        Platform::Delete<Crypto::EFR32OpaqueP256Keypair>(mCachedKey);
    }
}

CHIP_ERROR Efr32PsaOperationalKeystore::Init()
{
    // Todo: load key map from storage

    mCachedKey = Platform::New<Crypto::EFR32OpaqueP256Keypair>();
    if (!mCachedKey)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    mIsInitialized = true;
    return CHIP_NO_ERROR;
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

    // Scroll the map linearly
    for (size_t i = 0; i < SL_MATTER_MAX_STORED_OP_KEYS; i++)
    {
        if (fabricIndex == mKeyMapping[i])
        {
            return true;
        }
    }

    return false;
}

CHIP_ERROR Efr32PsaOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                              MutableByteSpan & outCertificateSigningRequest)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMAX_CSR_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Generate new key
    EFR32OpaqueKeyId id = 0;

    if (mPendingFabricIndex != kUndefinedFabricIndex)
    {
        // If we already have a pending key, delete it and put a new one in its place
        error = mPendingKeypair->GetKeyId(&id);
        if (error != CHIP_NO_ERROR)
        {
            ResetPendingKey();
            return error;
        }

        mPendingKeypair->Delete();
    }

    if (id == 0)
    {
        // Find empty slot in keymap
        for (size_t i = 0; i < SL_MATTER_MAX_STORED_OP_KEYS; i++)
        {
            if (mKeyMapping[i] == kUndefinedFabricIndex)
            {
                id = i + 1;
                break;
            }

            if (i >= SL_MATTER_MAX_STORED_OP_KEYS - 1)
            {
                ResetPendingKey();
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        if (!mPendingKeypair)
        {
            mPendingKeypair = Platform::New<Crypto::EFR32OpaqueP256Keypair>();
        }
    }

    // Create new key on the old or found key ID
    error = mPendingKeypair->Create(id, Crypto::EFR32OpaqueKeyUsages::ECDSA_P256_SHA256);
    if (error != CHIP_NO_ERROR)
    {
        // Try deleting and recreating this key since keys don't get wiped on factory erase yet
        mPendingKeypair->Delete();
        error = mPendingKeypair->Create(id, Crypto::EFR32OpaqueKeyUsages::ECDSA_P256_SHA256);
    }

    if (error != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return error;
    }

    // Set CSR and state
    size_t csrLength = outCertificateSigningRequest.size();
    error = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
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
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Efr32PsaOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Add key association to key map
    EFR32OpaqueKeyId id = 0;
    mPendingKeypair->GetKeyId(&id);

    if (id == 0)
    {
        ResetPendingKey();
        return CHIP_ERROR_INTERNAL;
    }

    if (mKeyMapping[id - 1] != kUndefinedFabricIndex)
    {
        ResetPendingKey();
        return CHIP_ERROR_INTERNAL;
    }

    mKeyMapping[id - 1] = fabricIndex;

    // Todo: persist key map

    // There's a good chance we'll need the key again soon
    mCachedKey->Load(id);

    mPendingKeypair           = nullptr;
    mIsPendingKeypairActive   = false;
    mPendingFabricIndex       = kUndefinedFabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Efr32PsaOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Figure out which key ID we're looking for
    EFR32OpaqueKeyId id = 0;
    for (size_t i = 0; i < SL_MATTER_MAX_STORED_OP_KEYS; i++)
    {
        if (mKeyMapping[i] == fabricIndex)
        {
            id = i + 1;

            // Reset the key mapping since we'll be deleting this key
            mKeyMapping[i] = kUndefinedFabricIndex;
            break;
        }

        if (i >= SL_MATTER_MAX_STORED_OP_KEYS - 1)
        {
            return CHIP_ERROR_INTERNAL;
        }
    }

    CHIP_ERROR error = CHIP_NO_ERROR;

    // TODO: Persist key map here

    // Check if key is cached
    EFR32OpaqueKeyId cachedId = 0;
    mCachedKey->GetKeyId(&cachedId);

    if (id == cachedId)
    {
        // Delete from persistent storage and unload
        mCachedKey->Delete();
        return CHIP_NO_ERROR;
    }

    // Load it for purposes of deletion
    error = mCachedKey->Load(id);
    if (error != CHIP_NO_ERROR && error != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        return CHIP_ERROR_INTERNAL;
    }

    mCachedKey->Delete();

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
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Check to see whether the key is an activated pending key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    // Figure out which key ID we're looking for
    EFR32OpaqueKeyId id = 0;
    for (size_t i = 0; i < SL_MATTER_MAX_STORED_OP_KEYS; i++)
    {
        if (mKeyMapping[i] == fabricIndex)
        {
            id = i + 1;
            break;
        }

        if (i >= SL_MATTER_MAX_STORED_OP_KEYS - 1)
        {
            return CHIP_ERROR_INTERNAL;
        }
    }

    // Check whether we have the key in cache
    EFR32OpaqueKeyId cachedId = 0;
    mCachedKey->GetKeyId(&cachedId);

    if (id == cachedId)
    {
        return mCachedKey->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    // If not, we need to recreate from the backend
    // Todo: figure out a way to construct the cache such that we can actually update it here
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
    Crypto::EFR32OpaqueP256Keypair * new_key = Platform::New<Crypto::EFR32OpaqueP256Keypair>();

    if (new_key != nullptr)
    {
        new_key->Create(0, Crypto::EFR32OpaqueKeyUsages::ECDH_P256);
    }

    return new_key;
}

void Efr32PsaOperationalKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<EFR32OpaqueP256Keypair>((Crypto::EFR32OpaqueP256Keypair *)keypair);
}

} // namespace chip
