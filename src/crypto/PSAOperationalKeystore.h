/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/CHIPCryptoPALPSA.h>
#include <crypto/OperationalKeystore.h>

namespace chip {
namespace Crypto {

class PSAOperationalKeystore final : public OperationalKeystore
{
public:
    bool HasPendingOpKeypair() const override;
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
    class PersistentP256Keypair : private P256Keypair
    {
    public:
        explicit PersistentP256Keypair(FabricIndex fabricIndex);
        ~PersistentP256Keypair() override;

        using P256Keypair::ECDSA_sign_msg;
        using P256Keypair::NewCertificateSigningRequest;
        using P256Keypair::Pubkey;

        psa_key_id_t GetKeyId() const;
        bool Exists() const;
        CHIP_ERROR Generate();
        CHIP_ERROR Destroy();
    };

    void ReleasePendingKeypair();

    PersistentP256Keypair * mPendingKeypair = nullptr;
    FabricIndex mPendingFabricIndex         = kUndefinedFabricIndex;
    bool mIsPendingKeypairActive            = false;
};

} // namespace Crypto
} // namespace chip
