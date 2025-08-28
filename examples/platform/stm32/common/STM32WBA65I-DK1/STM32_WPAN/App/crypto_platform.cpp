/*
 *    Copyright (c) 2021, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the Crypto platform callbacks into OpenThread and default/weak Crypto platform APIs.
 */

#include "openthread-core-config.h"

#include <string.h>

#include <mbedtls/aes.h>
#include <mbedtls/cmac.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>
#include <mbedtls/version.h>

//#include <openthread/instance.h>
#include <openthread/platform/crypto.h>
#include <openthread/platform/entropy.h>
#include <openthread/platform/time.h>

#include "common/code_utils.hpp"
#include "common/num_utils.hpp"
#include "common/debug.hpp"
#include "common/new.hpp"
#include "common/array.hpp"
#include "common/as_core_type.hpp"
#include "common/error.hpp"
#include "common/random.hpp"
#include "common/time_ticker.hpp"
#include "common/timer.hpp"
#include "common/uptime.hpp"

#include "crypto/mbedtls.hpp"
#include "config/crypto.h"
#include "crypto/ecdsa.hpp"
#include "crypto/hmac_sha256.hpp"
#include "crypto/storage.hpp"
//#include "instance/instance.hpp"

using namespace ot;
using namespace Crypto;

#if OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_MBEDTLS

//---------------------------------------------------------------------------------------------------------------------
// Default/weak implementation of crypto platform APIs

#if (!defined(MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES) && \
     (!defined(MBEDTLS_NO_PLATFORM_ENTROPY) || defined(MBEDTLS_HAVEGE_C) || defined(MBEDTLS_ENTROPY_HARDWARE_ALT)))
#define OT_MBEDTLS_STRONG_DEFAULT_ENTROPY_PRESENT
#endif

#if !OPENTHREAD_RADIO
static mbedtls_ctr_drbg_context sCtrDrbgContext;
static mbedtls_entropy_context  sEntropyContext;
#ifndef OT_MBEDTLS_STRONG_DEFAULT_ENTROPY_PRESENT
static constexpr uint16_t kEntropyMinThreshold = 16;
#endif
#endif

OT_TOOL_WEAK void otPlatCryptoInit(void)
{
    // Intentionally empty.
}

// AES  Implementation
OT_TOOL_WEAK otError otPlatCryptoAesInit(otCryptoContext *aContext)
{
    Error                error = kErrorNone;
    mbedtls_aes_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_aes_context), error = kErrorFailed);

    context = static_cast<mbedtls_aes_context *>(aContext->mContext);
    mbedtls_aes_init(context);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoAesSetKey(otCryptoContext *aContext, const otCryptoKey *aKey)
{
    Error                error = kErrorNone;
    mbedtls_aes_context *context;
    const LiteralKey     key(*static_cast<const Key *>(aKey));

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_aes_context), error = kErrorFailed);

    context = static_cast<mbedtls_aes_context *>(aContext->mContext);
    VerifyOrExit((mbedtls_aes_setkey_enc(context, key.GetBytes(), (key.GetLength() * kBitsPerByte)) == 0),
                 error = kErrorFailed);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoAesEncrypt(otCryptoContext *aContext, const uint8_t *aInput, uint8_t *aOutput)
{
    Error                error = kErrorNone;
    mbedtls_aes_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_aes_context), error = kErrorFailed);

    context = static_cast<mbedtls_aes_context *>(aContext->mContext);
    VerifyOrExit((mbedtls_aes_crypt_ecb(context, MBEDTLS_AES_ENCRYPT, aInput, aOutput) == 0), error = kErrorFailed);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoAesFree(otCryptoContext *aContext)
{
    Error                error = kErrorNone;
    mbedtls_aes_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_aes_context), error = kErrorFailed);

    context = static_cast<mbedtls_aes_context *>(aContext->mContext);
    mbedtls_aes_free(context);

exit:
    return error;
}

#if !OPENTHREAD_RADIO

// HMAC implementations
OT_TOOL_WEAK otError otPlatCryptoHmacSha256Init(otCryptoContext *aContext)
{
    Error                    error  = kErrorNone;
    const mbedtls_md_info_t *mdInfo = nullptr;
    mbedtls_md_context_t    *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_md_context_t), error = kErrorFailed);

    context = static_cast<mbedtls_md_context_t *>(aContext->mContext);
    mbedtls_md_init(context);
    mdInfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit((mbedtls_md_setup(context, mdInfo, 1) == 0), error = kErrorFailed);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Deinit(otCryptoContext *aContext)
{
    Error                 error = kErrorNone;
    mbedtls_md_context_t *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_md_context_t), error = kErrorFailed);

    context = static_cast<mbedtls_md_context_t *>(aContext->mContext);
    mbedtls_md_free(context);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Start(otCryptoContext *aContext, const otCryptoKey *aKey)
{
    Error                 error = kErrorNone;
    const LiteralKey      key(*static_cast<const Key *>(aKey));
    mbedtls_md_context_t *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_md_context_t), error = kErrorFailed);

    context = static_cast<mbedtls_md_context_t *>(aContext->mContext);
    VerifyOrExit((mbedtls_md_hmac_starts(context, key.GetBytes(), key.GetLength()) == 0), error = kErrorFailed);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength)
{
    Error                 error = kErrorNone;
    mbedtls_md_context_t *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_md_context_t), error = kErrorFailed);

    context = static_cast<mbedtls_md_context_t *>(aContext->mContext);
    VerifyOrExit((mbedtls_md_hmac_update(context, reinterpret_cast<const uint8_t *>(aBuf), aBufLength) == 0),
                 error = kErrorFailed);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHmacSha256Finish(otCryptoContext *aContext, uint8_t *aBuf, size_t aBufLength)
{
    OT_UNUSED_VARIABLE(aBufLength);

    Error                 error = kErrorNone;
    mbedtls_md_context_t *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_md_context_t), error = kErrorFailed);

    context = static_cast<mbedtls_md_context_t *>(aContext->mContext);
    VerifyOrExit((mbedtls_md_hmac_finish(context, aBuf) == 0), error = kErrorFailed);

exit:
    return error;
}

otError otPlatCryptoHkdfInit(otCryptoContext *aContext)
{
    Error error = kErrorNone;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(HmacSha256::Hash), error = kErrorFailed);

    new (aContext->mContext) HmacSha256::Hash();

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHkdfExpand(otCryptoContext *aContext,
                                            const uint8_t   *aInfo,
                                            uint16_t         aInfoLength,
                                            uint8_t         *aOutputKey,
                                            uint16_t         aOutputKeyLength)
{
    Error             error = kErrorNone;
    HmacSha256        hmac;
    HmacSha256::Hash  hash;
    uint8_t           iter = 0;
    uint16_t          copyLength;
    HmacSha256::Hash *prk;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(HmacSha256::Hash), error = kErrorFailed);

    prk = static_cast<HmacSha256::Hash *>(aContext->mContext);

    // The aOutputKey is calculated as follows [RFC5889]:
    //
    //   N = ceil( aOutputKeyLength / HashSize)
    //   T = T(1) | T(2) | T(3) | ... | T(N)
    //   aOutputKey is first aOutputKeyLength of T
    //
    // Where:
    //   T(0) = empty string (zero length)
    //   T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
    //   T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
    //   T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
    //   ...

    while (aOutputKeyLength > 0)
    {
        Key cryptoKey;

        cryptoKey.Set(prk->GetBytes(), sizeof(HmacSha256::Hash));
        hmac.Start(cryptoKey);

        if (iter != 0)
        {
            hmac.Update(hash);
        }

        hmac.Update(aInfo, aInfoLength);

        iter++;
        hmac.Update(iter);
        hmac.Finish(hash);

        copyLength = Min(aOutputKeyLength, static_cast<uint16_t>(sizeof(hash)));

        memcpy(aOutputKey, hash.GetBytes(), copyLength);
        aOutputKey += copyLength;
        aOutputKeyLength -= copyLength;
    }

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoHkdfExtract(otCryptoContext   *aContext,
                                             const uint8_t     *aSalt,
                                             uint16_t           aSaltLength,
                                             const otCryptoKey *aInputKey)
{
    Error             error = kErrorNone;
    HmacSha256        hmac;
    Key               cryptoKey;
    HmacSha256::Hash *prk;
    const LiteralKey  inputKey(*static_cast<const Key *>(aInputKey));

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(HmacSha256::Hash), error = kErrorFailed);

    prk = static_cast<HmacSha256::Hash *>(aContext->mContext);

    cryptoKey.Set(aSalt, aSaltLength);
    // PRK is calculated as HMAC-Hash(aSalt, aInputKey)
    hmac.Start(cryptoKey);
    hmac.Update(inputKey.GetBytes(), inputKey.GetLength());
    hmac.Finish(*prk);

exit:
    return error;
}

otError otPlatCryptoHkdfDeinit(otCryptoContext *aContext)
{
    Error             error = kErrorNone;
    HmacSha256::Hash *prk;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(HmacSha256::Hash), error = kErrorFailed);

    prk = static_cast<HmacSha256::Hash *>(aContext->mContext);
    prk->~Hash();
    aContext->mContext     = nullptr;
    aContext->mContextSize = 0;

exit:
    return error;
}

// SHA256 platform implementations
OT_TOOL_WEAK otError otPlatCryptoSha256Init(otCryptoContext *aContext)
{
    Error                   error = kErrorNone;
    mbedtls_sha256_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);

    context = static_cast<mbedtls_sha256_context *>(aContext->mContext);
    mbedtls_sha256_init(context);

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Deinit(otCryptoContext *aContext)
{
    Error                   error = kErrorNone;
    mbedtls_sha256_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_sha256_context), error = kErrorFailed);

    context = static_cast<mbedtls_sha256_context *>(aContext->mContext);
    mbedtls_sha256_free(context);
    aContext->mContext     = nullptr;
    aContext->mContextSize = 0;

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Start(otCryptoContext *aContext)
{
    Error                   error = kErrorNone;
    mbedtls_sha256_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_sha256_context), error = kErrorFailed);

    context = static_cast<mbedtls_sha256_context *>(aContext->mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    VerifyOrExit((mbedtls_sha256_starts(context, 0) == 0), error = kErrorFailed);
#else
    VerifyOrExit((mbedtls_sha256_starts_ret(context, 0) == 0), error = kErrorFailed);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Update(otCryptoContext *aContext, const void *aBuf, uint16_t aBufLength)
{
    Error                   error = kErrorNone;
    mbedtls_sha256_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_sha256_context), error = kErrorFailed);

    context = static_cast<mbedtls_sha256_context *>(aContext->mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    VerifyOrExit((mbedtls_sha256_update(context, reinterpret_cast<const uint8_t *>(aBuf), aBufLength) == 0),
                 error = kErrorFailed);
#else
    VerifyOrExit((mbedtls_sha256_update_ret(context, reinterpret_cast<const uint8_t *>(aBuf), aBufLength) == 0),
                 error = kErrorFailed);
#endif

exit:
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoSha256Finish(otCryptoContext *aContext, uint8_t *aHash, uint16_t aHashSize)
{
    OT_UNUSED_VARIABLE(aHashSize);

    Error                   error = kErrorNone;
    mbedtls_sha256_context *context;

    VerifyOrExit(aContext != nullptr, error = kErrorInvalidArgs);
    VerifyOrExit(aContext->mContextSize >= sizeof(mbedtls_sha256_context), error = kErrorFailed);

    context = static_cast<mbedtls_sha256_context *>(aContext->mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    VerifyOrExit((mbedtls_sha256_finish(context, aHash) == 0), error = kErrorFailed);
#else
    VerifyOrExit((mbedtls_sha256_finish_ret(context, aHash) == 0), error = kErrorFailed);
#endif

exit:
    return error;
}

#ifndef OT_MBEDTLS_STRONG_DEFAULT_ENTROPY_PRESENT

static int handleMbedtlsEntropyPoll(void *aData, unsigned char *aOutput, size_t aInLen, size_t *aOutLen)
{
    int rval = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;

    SuccessOrExit(otPlatEntropyGet(reinterpret_cast<uint8_t *>(aOutput), static_cast<uint16_t>(aInLen)));
    rval = 0;

    VerifyOrExit(aOutLen != nullptr);
    *aOutLen = aInLen;

exit:
    OT_UNUSED_VARIABLE(aData);
    return rval;
}

#endif // OT_MBEDTLS_STRONG_DEFAULT_ENTROPY_PRESENT

OT_TOOL_WEAK void otPlatCryptoRandomInit(void)
{
    mbedtls_entropy_init(&sEntropyContext);

#ifndef OT_MBEDTLS_STRONG_DEFAULT_ENTROPY_PRESENT
    mbedtls_entropy_add_source(&sEntropyContext, handleMbedtlsEntropyPoll, nullptr, kEntropyMinThreshold,
                               MBEDTLS_ENTROPY_SOURCE_STRONG);
#endif

    mbedtls_ctr_drbg_init(&sCtrDrbgContext);

    int rval = mbedtls_ctr_drbg_seed(&sCtrDrbgContext, mbedtls_entropy_func, &sEntropyContext, nullptr, 0);
    OT_ASSERT(rval == 0);
    OT_UNUSED_VARIABLE(rval);
}

OT_TOOL_WEAK void otPlatCryptoRandomDeinit(void)
{
    mbedtls_entropy_free(&sEntropyContext);
    mbedtls_ctr_drbg_free(&sCtrDrbgContext);
}

OT_TOOL_WEAK otError otPlatCryptoRandomGet(uint8_t *aBuffer, uint16_t aSize)
{
    return ot::Crypto::MbedTls::MapError(
        mbedtls_ctr_drbg_random(&sCtrDrbgContext, static_cast<unsigned char *>(aBuffer), static_cast<size_t>(aSize)));
}

#if OPENTHREAD_CONFIG_ECDSA_ENABLE

OT_TOOL_WEAK otError otPlatCryptoEcdsaGenerateKey(otPlatCryptoEcdsaKeyPair *aKeyPair)
{
    mbedtls_pk_context pk;
    int                ret;

    mbedtls_pk_init(&pk);

    ret = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    VerifyOrExit(ret == 0);

    ret = mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, mbedtls_pk_ec(pk), MbedTls::CryptoSecurePrng, nullptr);
    VerifyOrExit(ret == 0);

    ret = mbedtls_pk_write_key_der(&pk, aKeyPair->mDerBytes, OT_CRYPTO_ECDSA_MAX_DER_SIZE);
    VerifyOrExit(ret > 0);

    aKeyPair->mDerLength = static_cast<uint8_t>(ret);

    memmove(aKeyPair->mDerBytes, aKeyPair->mDerBytes + OT_CRYPTO_ECDSA_MAX_DER_SIZE - aKeyPair->mDerLength,
            aKeyPair->mDerLength);

exit:
    mbedtls_pk_free(&pk);

    return (ret >= 0) ? kErrorNone : MbedTls::MapError(ret);
}

OT_TOOL_WEAK otError otPlatCryptoEcdsaGetPublicKey(const otPlatCryptoEcdsaKeyPair *aKeyPair,
                                                   otPlatCryptoEcdsaPublicKey     *aPublicKey)
{
    Error                error = kErrorNone;
    mbedtls_pk_context   pk;
    mbedtls_ecp_keypair *keyPair;
    int                  ret;

    mbedtls_pk_init(&pk);

    VerifyOrExit(mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) == 0, error = kErrorFailed);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    VerifyOrExit(mbedtls_pk_parse_key(&pk, aKeyPair->mDerBytes, aKeyPair->mDerLength, nullptr, 0,
                                      MbedTls::CryptoSecurePrng, nullptr) == 0,
                 error = kErrorParse);
#else
    VerifyOrExit(mbedtls_pk_parse_key(&pk, aKeyPair->mDerBytes, aKeyPair->mDerLength, nullptr, 0) == 0,
                 error = kErrorParse);
#endif

    keyPair = mbedtls_pk_ec(pk);

    ret = mbedtls_mpi_write_binary(&keyPair->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X), aPublicKey->m8,
                                   Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    ret = mbedtls_mpi_write_binary(&keyPair->MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y),
                                   aPublicKey->m8 + Ecdsa::P256::kMpiSize, Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

exit:
    mbedtls_pk_free(&pk);
    return error;
}

OT_TOOL_WEAK otError otPlatCryptoEcdsaSign(const otPlatCryptoEcdsaKeyPair *aKeyPair,
                                           const otPlatCryptoSha256Hash   *aHash,
                                           otPlatCryptoEcdsaSignature     *aSignature)
{
    Error                 error = kErrorNone;
    mbedtls_pk_context    pk;
    mbedtls_ecp_keypair  *keypair;
    mbedtls_ecdsa_context ecdsa;
    mbedtls_mpi           r;
    mbedtls_mpi           s;
    int                   ret;

    mbedtls_pk_init(&pk);
    mbedtls_ecdsa_init(&ecdsa);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    VerifyOrExit(mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) == 0, error = kErrorFailed);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    VerifyOrExit(mbedtls_pk_parse_key(&pk, aKeyPair->mDerBytes, aKeyPair->mDerLength, nullptr, 0,
                                      MbedTls::CryptoSecurePrng, nullptr) == 0,
                 error = kErrorParse);
#else
    VerifyOrExit(mbedtls_pk_parse_key(&pk, aKeyPair->mDerBytes, aKeyPair->mDerLength, nullptr, 0) == 0,
                 error = kErrorParse);
#endif

    keypair = mbedtls_pk_ec(pk);

    ret = mbedtls_ecdsa_from_keypair(&ecdsa, keypair);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

#if (MBEDTLS_VERSION_NUMBER >= 0x02130000)
    ret = mbedtls_ecdsa_sign_det_ext(&ecdsa.MBEDTLS_PRIVATE(grp), &r, &s, &ecdsa.MBEDTLS_PRIVATE(d), aHash->m8,
                                     Sha256::Hash::kSize, MBEDTLS_MD_SHA256, MbedTls::CryptoSecurePrng, nullptr);
#else
    ret = mbedtls_ecdsa_sign_det(&ecdsa.MBEDTLS_PRIVATE(grp), &r, &s, &ecdsa.MBEDTLS_PRIVATE(d), aHash->m8,
                                 Sha256::Hash::kSize, MBEDTLS_MD_SHA256);
#endif
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    OT_ASSERT(mbedtls_mpi_size(&r) <= Ecdsa::P256::kMpiSize);

    ret = mbedtls_mpi_write_binary(&r, aSignature->m8, Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    ret = mbedtls_mpi_write_binary(&s, aSignature->m8 + Ecdsa::P256::kMpiSize, Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

exit:
    mbedtls_pk_free(&pk);
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_ecdsa_free(&ecdsa);

    return error;
}

OT_TOOL_WEAK otError otPlatCryptoEcdsaVerify(const otPlatCryptoEcdsaPublicKey *aPublicKey,
                                             const otPlatCryptoSha256Hash     *aHash,
                                             const otPlatCryptoEcdsaSignature *aSignature)
{
    Error                 error = kErrorNone;
    mbedtls_ecdsa_context ecdsa;
    mbedtls_mpi           r;
    mbedtls_mpi           s;
    int                   ret;

    mbedtls_ecdsa_init(&ecdsa);
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    ret = mbedtls_ecp_group_load(&ecdsa.MBEDTLS_PRIVATE(grp), MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    ret = mbedtls_mpi_read_binary(&ecdsa.MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X), aPublicKey->m8, Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));
    ret = mbedtls_mpi_read_binary(&ecdsa.MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y), aPublicKey->m8 + Ecdsa::P256::kMpiSize,
                                  Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));
    ret = mbedtls_mpi_lset(&ecdsa.MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Z), 1);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    ret = mbedtls_mpi_read_binary(&r, aSignature->m8, Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    ret = mbedtls_mpi_read_binary(&s, aSignature->m8 + Ecdsa::P256::kMpiSize, Ecdsa::P256::kMpiSize);
    VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

    ret = mbedtls_ecdsa_verify(&ecdsa.MBEDTLS_PRIVATE(grp), aHash->m8, Sha256::Hash::kSize, &ecdsa.MBEDTLS_PRIVATE(Q),
                               &r, &s);
    VerifyOrExit(ret == 0, error = kErrorSecurity);

exit:
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    mbedtls_ecdsa_free(&ecdsa);

    return error;
}

#endif // #if OPENTHREAD_CONFIG_ECDSA_ENABLE

#endif // #if !OPENTHREAD_RADIO

#if OPENTHREAD_FTD

OT_TOOL_WEAK otError otPlatCryptoPbkdf2GenerateKey(const uint8_t *aPassword,
                                                   uint16_t       aPasswordLen,
                                                   const uint8_t *aSalt,
                                                   uint16_t       aSaltLen,
                                                   uint32_t       aIterationCounter,
                                                   uint16_t       aKeyLen,
                                                   uint8_t       *aKey)
{
#if (MBEDTLS_VERSION_NUMBER >= 0x03050000)
    const size_t kBlockSize = MBEDTLS_CMAC_MAX_BLOCK_SIZE;
#else
    const size_t kBlockSize = MBEDTLS_CIPHER_BLKSIZE_MAX;
#endif
    uint8_t  prfInput[OT_CRYPTO_PBDKF2_MAX_SALT_SIZE + 4]; // Salt || INT(), for U1 calculation
    long     prfOne[kBlockSize / sizeof(long)];
    long     prfTwo[kBlockSize / sizeof(long)];
    long     keyBlock[kBlockSize / sizeof(long)];
    uint32_t blockCounter = 0;
    uint8_t *key          = aKey;
    uint16_t keyLen       = aKeyLen;
    uint16_t useLen       = 0;
    Error    error        = kErrorNone;
    int      ret;

    OT_ASSERT(aSaltLen <= sizeof(prfInput));
    memcpy(prfInput, aSalt, aSaltLen);
    OT_ASSERT(aIterationCounter % 2 == 0);
    aIterationCounter /= 2;

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    // limit iterations to avoid OSS-Fuzz timeouts
    aIterationCounter = 2;
#endif

    while (keyLen)
    {
        ++blockCounter;
        prfInput[aSaltLen + 0] = static_cast<uint8_t>(blockCounter >> 24);
        prfInput[aSaltLen + 1] = static_cast<uint8_t>(blockCounter >> 16);
        prfInput[aSaltLen + 2] = static_cast<uint8_t>(blockCounter >> 8);
        prfInput[aSaltLen + 3] = static_cast<uint8_t>(blockCounter);

        // Calculate U_1
        ret = mbedtls_aes_cmac_prf_128(aPassword, aPasswordLen, prfInput, aSaltLen + 4,
                                       reinterpret_cast<uint8_t *>(keyBlock));
        VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

        // Calculate U_2
        ret = mbedtls_aes_cmac_prf_128(aPassword, aPasswordLen, reinterpret_cast<const uint8_t *>(keyBlock), kBlockSize,
                                       reinterpret_cast<uint8_t *>(prfOne));
        VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

        for (uint32_t j = 0; j < kBlockSize / sizeof(long); ++j)
        {
            keyBlock[j] ^= prfOne[j];
        }

        for (uint32_t i = 1; i < aIterationCounter; ++i)
        {
            // Calculate U_{2 * i - 1}
            ret = mbedtls_aes_cmac_prf_128(aPassword, aPasswordLen, reinterpret_cast<const uint8_t *>(prfOne),
                                           kBlockSize, reinterpret_cast<uint8_t *>(prfTwo));
            VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));
            // Calculate U_{2 * i}
            ret = mbedtls_aes_cmac_prf_128(aPassword, aPasswordLen, reinterpret_cast<const uint8_t *>(prfTwo),
                                           kBlockSize, reinterpret_cast<uint8_t *>(prfOne));
            VerifyOrExit(ret == 0, error = MbedTls::MapError(ret));

            for (uint32_t j = 0; j < kBlockSize / sizeof(long); ++j)
            {
                keyBlock[j] ^= prfOne[j] ^ prfTwo[j];
            }
        }

        useLen = Min(keyLen, static_cast<uint16_t>(kBlockSize));
        memcpy(key, keyBlock, useLen);
        key += useLen;
        keyLen -= useLen;
    }

exit:
    return error;
}

#endif // #if OPENTHREAD_FTD

#elif OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_PSA

#if !OPENTHREAD_RADIO
#if OPENTHREAD_CONFIG_ECDSA_ENABLE

OT_TOOL_WEAK otError otPlatCryptoEcdsaGenerateKey(otPlatCryptoEcdsaKeyPair *aKeyPair)
{
    OT_UNUSED_VARIABLE(aKeyPair);

    return OT_ERROR_NOT_CAPABLE;
}

OT_TOOL_WEAK otError otPlatCryptoEcdsaGetPublicKey(const otPlatCryptoEcdsaKeyPair *aKeyPair,
                                                   otPlatCryptoEcdsaPublicKey     *aPublicKey)
{
    OT_UNUSED_VARIABLE(aKeyPair);
    OT_UNUSED_VARIABLE(aPublicKey);

    return OT_ERROR_NOT_CAPABLE;
}

OT_TOOL_WEAK otError otPlatCryptoEcdsaSign(const otPlatCryptoEcdsaKeyPair *aKeyPair,
                                           const otPlatCryptoSha256Hash   *aHash,
                                           otPlatCryptoEcdsaSignature     *aSignature)
{
    OT_UNUSED_VARIABLE(aKeyPair);
    OT_UNUSED_VARIABLE(aHash);
    OT_UNUSED_VARIABLE(aSignature);

    return OT_ERROR_NOT_CAPABLE;
}

OT_TOOL_WEAK otError otPlatCryptoEcdsaVerify(const otPlatCryptoEcdsaPublicKey *aPublicKey,
                                             const otPlatCryptoSha256Hash     *aHash,
                                             const otPlatCryptoEcdsaSignature *aSignature)

{
    OT_UNUSED_VARIABLE(aPublicKey);
    OT_UNUSED_VARIABLE(aHash);
    OT_UNUSED_VARIABLE(aSignature);

    return OT_ERROR_NOT_CAPABLE;
}
#endif // #if OPENTHREAD_CONFIG_ECDSA_ENABLE

#endif // #if !OPENTHREAD_RADIO

#if OPENTHREAD_FTD

OT_TOOL_WEAK otError otPlatCryptoPbkdf2GenerateKey(const uint8_t *aPassword,
                                                   uint16_t       aPasswordLen,
                                                   const uint8_t *aSalt,
                                                   uint16_t       aSaltLen,
                                                   uint32_t       aIterationCounter,
                                                   uint16_t       aKeyLen,
                                                   uint8_t       *aKey)
{
    OT_UNUSED_VARIABLE(aPassword);
    OT_UNUSED_VARIABLE(aPasswordLen);
    OT_UNUSED_VARIABLE(aSalt);
    OT_UNUSED_VARIABLE(aSaltLen);
    OT_UNUSED_VARIABLE(aIterationCounter);
    OT_UNUSED_VARIABLE(aKeyLen);
    OT_UNUSED_VARIABLE(aKey);

    return OT_ERROR_NOT_CAPABLE;
}

#endif // #if OPENTHREAD_FTD

#endif // #if OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_MBEDTLS
