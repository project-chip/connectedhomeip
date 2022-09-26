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

#include "CHIPCryptoPALHsm_SE05X_utils.h"
#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#if ENABLE_HSM_GENERATE_EC_KEY

namespace chip {

class PersistentStorageOperationalKeystoreHSM : public Crypto::OperationalKeystore
{
public:
    PersistentStorageOperationalKeystoreHSM() = default;
    virtual ~PersistentStorageOperationalKeystoreHSM() { Finish(); }

    // Non-copyable
    PersistentStorageOperationalKeystoreHSM(PersistentStorageOperationalKeystoreHSM const &) = delete;
    void operator=(PersistentStorageOperationalKeystoreHSM const &) = delete;

    /**
     * @brief Initialize the Operational Keystore for HSM.
     *
     * @param storage Pointer to persistent storage delegate to use. Must outlive this instance.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if already initialized
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        IgnoreUnusedVariable(storage);
        mPendingFabricIndex     = kUndefinedFabricIndex;
        mPendingKeypair         = nullptr;
        mIsPendingKeypairActive = false;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Finalize the keystore, so that subsequent operations fail
     */
    void Finish() { ResetPendingKey(); }

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
    void ResetPendingSlot();

    void ResetPendingKey()
    {
        if (mPendingKeypair != nullptr)
        {
            ResetPendingSlot();
            Platform::Delete(mPendingKeypair);
        }
        mPendingKeypair         = nullptr;
        mIsPendingKeypairActive = false;
        mPendingFabricIndex     = kUndefinedFabricIndex;
    }

    // This pending fabric index is `kUndefinedFabricIndex` if there isn't a pending keypair override for a given fabric.
    FabricIndex mPendingFabricIndex          = kUndefinedFabricIndex;
    Crypto::P256KeypairHSM * mPendingKeypair = nullptr;
    bool mIsPendingKeypairActive             = false;
};

} // namespace chip

#endif //#if ENABLE_HSM_GENERATE_EC_KEY
