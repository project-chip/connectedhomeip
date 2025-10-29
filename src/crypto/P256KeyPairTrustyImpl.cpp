/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 *    @file
 *      Trusty OS based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"
#include "CHIPCryptoPALOpenSSL.h"

#include <trusty_matter.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPArgParser.hpp>

static uint64_t mP256_Handler;
static uint8_t mFabricIndex;

namespace chip {
namespace Crypto {

static matter::TrustyMatter & GetTrustyMatter()
{
    static matter::TrustyMatter instance;
    return instance;
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t sig_size  = 0;
    int rc           = 0;

    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    uint8_t sig[kP256_ECDSA_Signature_Length_Raw];
    memset(&digest[0], 0, sizeof(digest));
    memset(&sig[0], 0, sizeof(sig));

    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    static_assert(P256ECDSASignature::Capacity() >= kP256_ECDSA_Signature_Length_Raw, "P256ECDSASignature must be large enough");
    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    rc = GetTrustyMatter().P256KeypairECSignMsg(mP256_Handler, digest, kSHA256_Hash_Length, sig, sig_size);
    VerifyOrExit(rc == MATTER_ERROR_OK, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(sig_size == kP256_ECDSA_Signature_Length_Raw, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    memcpy(out_signature.Bytes() + 0u, sig, sig_size);

exit:
    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result;
    size_t out_buf_length = 0;

    result = GetTrustyMatter().P256KeypairECDH_derive_secret(mP256_Handler, Uint8::to_const_uchar(remote_public_key),
                                                             remote_public_key.Length(), out_secret.Bytes(), out_buf_length);
    VerifyOrExit(result == MATTER_ERROR_OK, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((out_buf_length > 0), error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(out_buf_length));

exit:
    return error;
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t public_key[kP256_PublicKey_Length];
    int rc = 0;

    rc = GetTrustyMatter().P256KeypairInitialize(mP256_Handler, mFabricIndex, public_key);
    VerifyOrExit(rc == MATTER_ERROR_OK, error = CHIP_ERROR_INTERNAL);

    memcpy(Uint8::to_uchar(mPublicKey), public_key, kP256_PublicKey_Length);

    mInitialized = true;

exit:
    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t privkey[kP256_PrivateKey_Length];
    int rc = 0;

    rc = GetTrustyMatter().P256KeypairSerialize(mP256_Handler, privkey);
    VerifyOrExit(rc == MATTER_ERROR_OK, error = CHIP_ERROR_INTERNAL);

    {
        size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
        Encoding::BufferWriter bbuf(output.Bytes(), len);
        bbuf.Put(mPublicKey, mPublicKey.Length());
        bbuf.Put(privkey, sizeof(privkey));
        VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);
        output.SetLength(bbuf.Needed());
    }

exit:
    ClearSecretData(privkey, sizeof(privkey));
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());
    int rc = 0;

    uint8_t * pubkey  = input.Bytes();
    uint8_t * privkey = input.Bytes() + mPublicKey.Length();

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    rc = GetTrustyMatter().P256KeypairDeserialize(mP256_Handler, pubkey, mPublicKey.Length(), privkey, kP256_PrivateKey_Length);
    VerifyOrExit(rc == MATTER_ERROR_OK, error = CHIP_ERROR_INTERNAL);

    mInitialized = true;

exit:
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        GetTrustyMatter().P256KeypairDestroy(mP256_Handler);
        mInitialized  = false;
        mP256_Handler = 0;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    if (GetTrustyMatter().P256KeypairNewCSR(mP256_Handler, out_csr, csr_length) != MATTER_ERROR_OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
