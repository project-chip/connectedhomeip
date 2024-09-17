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

/**
 *    @file
 *          Platform-specific implementation of the persistent operational keystore for K32W1
 */

#pragma once

#include <crypto/OperationalKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include "sss_crypto.h"

namespace chip {

#define SSS_KEY_PAIR_BLOB_SIZE 120

typedef Crypto::SensitiveDataBuffer<SSS_KEY_PAIR_BLOB_SIZE> P256SerializedKeypairSSS;

class P256KeypairSSS : public Crypto::P256Keypair
{
public:
    /**
     * @brief Export an encrypted blob.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ExportBlob(P256SerializedKeypairSSS & output) const;

    /**
     * @brief Import an encrypted blob.
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR ImportBlob(P256SerializedKeypairSSS & input);
};

/**
 * @brief OperationalKeystore implementation making use of PersistentStorageDelegate
 *        to load/store keypairs. This is the legacy behavior of `FabricTable` prior
 *        to refactors to use `OperationalKeystore` and exists as a baseline example
 *        of how to use the interface.
 *
 */
class PersistentStorageOpKeystoreS200 : public Crypto::OperationalKeystore
{
public:
    PersistentStorageOpKeystoreS200() = default;
    virtual ~PersistentStorageOpKeystoreS200() { Finish(); }

    // Non-copyable
    PersistentStorageOpKeystoreS200(PersistentStorageOpKeystoreS200 const &) = delete;
    void operator=(PersistentStorageOpKeystoreS200 const &)                  = delete;

    /**
     * @brief Initialize the Operational Keystore to map to a given storage delegate.
     *
     * @param storage Pointer to persistent storage delegate to use. Must outlive this instance.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if already initialized
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
        mPendingFabricIndex       = kUndefinedFabricIndex;
        mIsExternallyOwnedKeypair = false;
        mStorage                  = storage;
        mPendingKeypair           = nullptr;
        mIsPendingKeypairActive   = false;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Finalize the keystore, so that subsequent operations fail
     */
    void Finish()
    {
        VerifyOrReturn(mStorage != nullptr);

        ResetPendingKey();
        mStorage = nullptr;
    }

    bool HasPendingOpKeypair() const override { return (mPendingKeypair != nullptr); }

    bool HasOpKeypairForFabric(FabricIndex fabricIndex) const override;
    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override;
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey) override;
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
    void RevertPendingKeypair() override;
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override;
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override;
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override;

protected:
    void ResetPendingKey()
    {
        if (!mIsExternallyOwnedKeypair && (mPendingKeypair != nullptr))
        {
            Platform::Delete(mPendingKeypair);
        }
        if (mCachedKeypair != nullptr)
        {
            Platform::Delete(mCachedKeypair);
        }
        mPendingKeypair           = nullptr;
        mCachedKeypair            = nullptr;
        mIsExternallyOwnedKeypair = false;
        mIsPendingKeypairActive   = false;
        mPendingFabricIndex       = kUndefinedFabricIndex;
        mCachedFabricIndex        = kUndefinedFabricIndex;
    }

    PersistentStorageDelegate * mStorage = nullptr;

    // This pending fabric index is `kUndefinedFabricIndex` if there isn't a pending keypair override for a given fabric.
    FabricIndex mPendingFabricIndex  = kUndefinedFabricIndex;
    P256KeypairSSS * mPendingKeypair = nullptr;
    bool mIsPendingKeypairActive     = false;

    // Optimize loading the keyblob from storage all the time
    mutable P256KeypairSSS * mCachedKeypair = nullptr;
    mutable FabricIndex mCachedFabricIndex  = kUndefinedFabricIndex;

    // If overridding NewOpKeypairForFabric method in a subclass, set this to true in
    // `NewOpKeypairForFabric` if the mPendingKeypair should not be deleted when no longer in use.
    bool mIsExternallyOwnedKeypair = false;
};

} // namespace chip
