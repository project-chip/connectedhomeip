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
 *
 *    Copyright 2023 NXP
 */

#pragma once

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include <trusty_matter.h>

using namespace matter;

namespace chip {
namespace Trusty {

/**
 * @brief OperationalKeystore implementation making use of Trusty secure storage
 *        to load/store keypairs.
 *
 */
class PersistentStorageOperationalKeystoreTrusty : public Crypto::OperationalKeystore
{
public:
    PersistentStorageOperationalKeystoreTrusty() = default;
    virtual ~PersistentStorageOperationalKeystoreTrusty() { Finish(); }

    // Non-copyable
    PersistentStorageOperationalKeystoreTrusty(PersistentStorageOperationalKeystoreTrusty const &) = delete;
    void operator=(PersistentStorageOperationalKeystoreTrusty const &) = delete;

    /**
     * @brief Finalize the keystore, so that subsequent operations fail
     */
    void Finish()
    {
        ResetPendingKey();
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
        mPendingKeypair           = nullptr;
        mIsExternallyOwnedKeypair = false;
        mIsPendingKeypairActive   = false;
        mPendingFabricIndex       = kUndefinedFabricIndex;
    }

    FabricIndex mPendingFabricIndex       = kUndefinedFabricIndex;
    Crypto::P256Keypair * mPendingKeypair = nullptr;
    bool mIsPendingKeypairActive          = false;

    // If overridding NewOpKeypairForFabric method in a subclass, set this to true in
    // `NewOpKeypairForFabric` if the mPendingKeypair should not be deleted when no longer in use.
    bool mIsExternallyOwnedKeypair = false;

private:
    mutable TrustyMatter trusty_matter;
};

} // namespace Trusty
} // namespace chip
