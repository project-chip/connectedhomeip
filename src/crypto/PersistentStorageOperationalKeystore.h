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

#pragma once

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace chip {

/**
 * @brief OperationalKeystore implementation making use of PersistentStorageDelegate
 *        to load/store keypairs. This is the legacy behavior of `FabricTable` prior
 *        to refactors to use `OperationalKeystore` and exists as a baseline example
 *        of how to use the interface.
 *
 * WARNING: Ensure that any implementation that uses this one as a starting point
 *          DOES NOT have the raw key material (in usable form) passed up/down to
 *          direct storage APIs that may make copies on heap/stack without sanitization.
 */
class PersistentStorageOperationalKeystore : public Crypto::OperationalKeystore
{
public:
    PersistentStorageOperationalKeystore() = default;
    virtual ~PersistentStorageOperationalKeystore() { Finish(); }

    // Non-copyable
    PersistentStorageOperationalKeystore(PersistentStorageOperationalKeystore const &) = delete;
    void operator=(PersistentStorageOperationalKeystore const &)                       = delete;

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
    CHIP_ERROR ExportOpKeypairForFabric(FabricIndex fabricIndex, Crypto::P256SerializedKeypair & outKeypair) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
    void RevertPendingKeypair() override;
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override;
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override;
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override;
    CHIP_ERROR MigrateOpKeypairForFabric(FabricIndex fabricIndex, OperationalKeystore & operationalKeystore) const override;

protected:
    void ResetPendingKey()
    {
        if (!mIsExternallyOwnedKeypair && (mPendingKeypair != nullptr))
        {
            Platform::Delete(mPendingKeypair);
        }
        mPendingKeypair           = nullptr;
        mIsExternallyOwnedKeypair = false;
        mIsPendingKeypairActive   = false;
        mPendingFabricIndex       = kUndefinedFabricIndex;
    }

    PersistentStorageDelegate * mStorage = nullptr;

    // This pending fabric index is `kUndefinedFabricIndex` if there isn't a pending keypair override for a given fabric.
    FabricIndex mPendingFabricIndex       = kUndefinedFabricIndex;
    Crypto::P256Keypair * mPendingKeypair = nullptr;
    bool mIsPendingKeypairActive          = false;

    // If overridding NewOpKeypairForFabric method in a subclass, set this to true in
    // `NewOpKeypairForFabric` if the mPendingKeypair should not be deleted when no longer in use.
    bool mIsExternallyOwnedKeypair = false;
};

} // namespace chip
