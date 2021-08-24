/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import "CHIPKeypair.h"

#import "CHIPError_Internal.h"
#include <crypto/CHIPCryptoPAL.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPP256KeypairBridge : public chip::Crypto::P256KeypairBase
{
public:
    ~CHIPP256KeypairBridge(){};

    CHIP_ERROR Init(id<CHIPKeypair> keypair);

    bool HasKeypair() const { return mKeypair != nil; };

    CHIP_ERROR Initialize() override;

    CHIP_ERROR Serialize(chip::Crypto::P256SerializedKeypair & output) const override;

    CHIP_ERROR Deserialize(chip::Crypto::P256SerializedKeypair & input) override;

    CHIP_ERROR NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) override;

    CHIP_ERROR ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, chip::Crypto::P256ECDSASignature & out_signature) override;

    CHIP_ERROR ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, chip::Crypto::P256ECDSASignature & out_signature) override;

    CHIP_ERROR ECDH_derive_secret(const chip::Crypto::P256PublicKey & remote_public_key,
                                  chip::Crypto::P256ECDHDerivedSecret & out_secret) const override;

    const chip::Crypto::P256PublicKey & Pubkey() const override { return mPubkey; };

private:
    id<CHIPKeypair> mKeypair;
    chip::Crypto::P256PublicKey mPubkey;

    CHIP_ERROR setPubkey();
};

NS_ASSUME_NONNULL_END
