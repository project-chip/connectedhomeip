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

#include <platform/Darwin/CHIPP256KeypairNativeBridge.h>

using namespace chip::Crypto;

CHIPP256KeypairNativeBridge::~CHIPP256KeypairNativeBridge() {}

CHIP_ERROR CHIPP256KeypairNativeBridge::Initialize()
{
    return mKeypairBase.Initialize();
}

CHIP_ERROR CHIPP256KeypairNativeBridge::Serialize(P256SerializedKeypair & output) const
{
    return mKeypairBase.Serialize(output);
}

CHIP_ERROR CHIPP256KeypairNativeBridge::Deserialize(P256SerializedKeypair & input)
{
    return mKeypairBase.Deserialize(input);
}

CHIP_ERROR CHIPP256KeypairNativeBridge::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature)
{
    return mKeypairBase.ECDSA_sign_msg(msg, msg_length, out_signature);
}

CHIP_ERROR CHIPP256KeypairNativeBridge::ECDSA_sign_hash(const uint8_t * hash, size_t hash_length,
                                                        P256ECDSASignature & out_signature)
{
    return mKeypairBase.ECDSA_sign_hash(hash, hash_length, out_signature);
}

CHIP_ERROR CHIPP256KeypairNativeBridge::ECDH_derive_secret(const P256PublicKey & remote_public_key,
                                                           P256ECDHDerivedSecret & out_secret) const
{
    return mKeypairBase.ECDH_derive_secret(remote_public_key, out_secret);
}

CHIP_ERROR CHIPP256KeypairNativeBridge::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length)
{
    return mKeypairBase.NewCertificateSigningRequest(csr, csr_length);
}
