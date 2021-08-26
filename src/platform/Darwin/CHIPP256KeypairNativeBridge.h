/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>

namespace chip {
namespace Crypto {

// This class allows bridging a Darwin P256KeypairBase implementation to
// the expected P256Keypair implementation in libChip.
//
// TODO: The Darwin layer is not able to directly extend P256Keypair for some reason.
// Remove this wrapper when that is figured out.
class CHIPP256KeypairNativeBridge : public P256Keypair
{
public:
    CHIPP256KeypairNativeBridge(P256KeypairBase & baseKeypair) : mKeypairBase(baseKeypair) {}

    ~CHIPP256KeypairNativeBridge();

    CHIP_ERROR Initialize() override;

    CHIP_ERROR Serialize(P256SerializedKeypair & output) const override;

    CHIP_ERROR Deserialize(P256SerializedKeypair & input) override;

    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) override;

    CHIP_ERROR ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, P256ECDSASignature & out_signature) override;

    CHIP_ERROR ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const override;

    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) override;

    const P256PublicKey & Pubkey() const override { return mKeypairBase.Pubkey(); }

private:
    P256KeypairBase & mKeypairBase;
};

} // namespace Crypto
} // namespace chip
