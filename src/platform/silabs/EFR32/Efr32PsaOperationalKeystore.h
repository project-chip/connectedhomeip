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
#include <lib/core/CHIPConfig.h>

#include <psa/crypto.h>

#include "Efr32OpaqueKeypair.h"
#include <platform/CHIPDeviceLayer.h>

// Set SL_MATTER_MAX_STORED_OP_KEYS to the preferred size of the mapping table
// between fabric IDs and opaque key indices. It can not be less than
// CHIP_CONFIG_MAX_FABRICS + 1 (since there would be too few map elements to
// support all fabrics the application wants to support in addition to an extra
// pending key), but can be larger in case a consistent on-disk size of the map
// is required.
#ifndef SL_MATTER_MAX_STORED_OP_KEYS
#define SL_MATTER_MAX_STORED_OP_KEYS (CHIP_CONFIG_MAX_FABRICS + 1)
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * @brief OperationalKeystore implementation making use of the EFR32 SDK-provided
 *        storage mechanisms to load/store keypairs.
 *
 * WARNING: Ensure that any implementation that uses this one as a starting point
 *          DOES NOT have the raw key material (in usable form) passed up/down to
 *          direct storage APIs that may make copies on heap/stack without sanitization.
 */
class Efr32PsaOperationalKeystore : public chip::Crypto::OperationalKeystore
{
public:
    Efr32PsaOperationalKeystore(){};
    virtual ~Efr32PsaOperationalKeystore() override;

    // Non-copyable
    Efr32PsaOperationalKeystore(Efr32PsaOperationalKeystore const &) = delete;
    void operator=(Efr32PsaOperationalKeystore const &) = delete;

    /**
     * @brief Initialize the Operational Keystore
     */
    CHIP_ERROR Init();

    bool HasPendingOpKeypair() const override { return (mPendingKeypair != nullptr); }

    bool HasOpKeypairForFabric(FabricIndex fabricIndex) const override;
    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override;
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const chip::Crypto::P256PublicKey & nocPublicKey) override;
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
    void RevertPendingKeypair() override;
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 chip::Crypto::P256ECDSASignature & outSignature) const override;
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override;
    void ReleaseEphemeralKeypair(chip::Crypto::P256Keypair * keypair) override;

protected:
    // The keymap maps PSA Crypto persistent key ID offsets against fabric IDs.
    // The keymap is persisted in NVM3, and the keys are stored through the PSA
    // API.
    FabricIndex * mKeyMap = nullptr;
    size_t mKeyMapSize    = 0;

    // The key cache is to avoid having to reconstruct keys from the storage
    // backend all the time (since it is rather slow).
    EFR32OpaqueP256Keypair * mCachedKey = nullptr;

    // This pending fabric index is `kUndefinedFabricIndex` if there isn't a
    // pending keypair override for a given fabric.
    FabricIndex mPendingFabricIndex          = kUndefinedFabricIndex;
    EFR32OpaqueP256Keypair * mPendingKeypair = nullptr;
    bool mIsPendingKeypairActive             = false;
    bool mIsInitialized                      = false;

private:
    void ResetPendingKey()
    {
        if (mPendingKeypair != nullptr)
        {
            mPendingKeypair->Delete();
            Platform::Delete(mPendingKeypair);
        }
        mPendingKeypair         = nullptr;
        mIsPendingKeypairActive = false;
        mPendingFabricIndex     = kUndefinedFabricIndex;
    }

    void Deinit()
    {
        ResetPendingKey();

        if (mCachedKey != nullptr)
        {
            Platform::Delete<EFR32OpaqueP256Keypair>(mCachedKey);
            mCachedKey = nullptr;
        }

        if (mKeyMap != nullptr)
        {
            Platform::MemoryFree(mKeyMap);
            mKeyMap     = nullptr;
            mKeyMapSize = 0;
        }

        mIsInitialized = false;
    }

    /**
     * @brief Find the opaque key ID stored in the map for a given
     *        fabric ID.
     *
     * @param fabricIndex The fabric index to find the opaque key ID for.
     *                    Can also be kUndefinedFabricIndex to find the first
     *                    unoccupied key ID.
     *
     * @return a valid key ID on match, or kEFR32OpaqueKeyIdUnknown if no
     *         match is found.
     */
    EFR32OpaqueKeyId FindKeyIdForFabric(FabricIndex fabricIndex) const;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
