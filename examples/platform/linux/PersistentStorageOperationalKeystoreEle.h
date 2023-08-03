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

#include "hsm_api.h"

namespace chip {
namespace ele {

/**
 * @brief OperationalKeystore implementation making use of ELE secure storage
 *        to load/store keypairs.
 *
 */
class PersistentStorageOperationalKeystoreEle : public Crypto::OperationalKeystore
{
public:
    PersistentStorageOperationalKeystoreEle();
    ~PersistentStorageOperationalKeystoreEle();

    // Non-copyable
    PersistentStorageOperationalKeystoreEle(PersistentStorageOperationalKeystoreEle const &) = delete;
    void operator=(PersistentStorageOperationalKeystoreEle const &) = delete;

    bool HasPendingOpKeypair() const override { return (mPendingFabricIndex != kUndefinedFabricIndex); }

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
    FabricIndex mPendingFabricIndex       = kUndefinedFabricIndex;
    bool mIsPendingKeypairActive          = false;

    // If overridding NewOpKeypairForFabric method in a subclass, set this to true in
    // `NewOpKeypairForFabric` if the mPendingKeypair should not be deleted when no longer in use.
    bool mIsExternallyOwnedKeypair = false;

private:
    void ResetPendingKey(bool delete_key);
    hsm_err_t EleDeleteKey(uint32_t keyId);
    CHIP_ERROR EleGenerateCSR(uint32_t keyId, uint8_t * csr, size_t &csrLength);
    hsm_err_t EleSignMessage(uint32_t keyId,
                             const uint8_t *msg, size_t msgSize,
                             uint8_t *sig, size_t sigSize) const;

    hsm_hdl_t hsm_session_hdl = 0;
    hsm_hdl_t key_store_hdl = 0;
    hsm_hdl_t key_mgmt_hdl = 0;
};

} // namespace ele
} // namespace chip
