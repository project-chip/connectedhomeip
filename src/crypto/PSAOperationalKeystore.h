/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
