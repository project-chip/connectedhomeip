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

#include <psa/crypto.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/EFR32/EFR32Config.h>
#include "Efr32OpaqueKeypair.h"

#ifndef SL_MATTER_MAX_STORED_OP_KEYS
#define SL_MATTER_MAX_STORED_OP_KEYS (kMaxValidFabricIndex - kMinValidFabricIndex + 1)
#endif

namespace chip {

/**
 * @brief OperationalKeystore implementation making use of the EFR32 SDK-provided
 *        storage mechanisms to load/store keypairs.
 *
 * WARNING: Ensure that any implementation that uses this one as a starting point
 *          DOES NOT have the raw key material (in usable form) passed up/down to
 *          direct storage APIs that may make copies on heap/stack without sanitization.
 */
class Efr32PsaOperationalKeystore : public Crypto::OperationalKeystore
{
public:
    Efr32PsaOperationalKeystore() {};
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
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey) override;
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override;
    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override;
    void RevertPendingKeypair() override;
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override;
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override;
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override;

protected:
    // The keymap maps PSA Crypto persistent key ID offsets against fabric IDs.
    // The keymap is persisted in NVM3, and the keys are stored through the PSA
    // API.
    FabricIndex mKeyMapping[SL_MATTER_MAX_STORED_OP_KEYS] = { kUndefinedFabricIndex };

    // The key cache is to avoid having to reconstruct keys from the storage
    // backend all the time (since it is rather slow).
    Crypto::EFR32OpaqueP256Keypair * mCachedKey = nullptr;

    // This pending fabric index is `kUndefinedFabricIndex` if there isn't a
    // pending keypair override for a given fabric.
    FabricIndex mPendingFabricIndex                     = kUndefinedFabricIndex;
    Crypto::EFR32OpaqueP256Keypair * mPendingKeypair    = nullptr;
    bool mIsPendingKeypairActive                        = false;
    bool mIsInitialized                                 = false;

private:
    void ResetPendingKey()
    {
        if (mPendingKeypair != nullptr)
        {
            mPendingKeypair->Delete();
            Platform::Delete(mPendingKeypair);
        }
        mPendingKeypair           = nullptr;
        mIsPendingKeypairActive   = false;
        mPendingFabricIndex       = kUndefinedFabricIndex;
    }
};

} // namespace chip
