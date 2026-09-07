/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *      Custom SPAKE2P implementation for PSA Crypto
 */

#include "SecLib_ecp256.h"
#include <crypto/CHIPCryptoPAL.h>
namespace chip {
namespace Crypto {

typedef struct Spake2p_Context
{
    ecp256Point_t M;
    ecp256Point_t N;
    ecp256Point_t X;
    ecp256Point_t Y;
    ecp256Point_t L;
    ecp256Point_t Z;
    ecp256Point_t V;

    big_int256_t w0;
    big_int256_t w1;
    big_int256_t xy;
    big_int256_t tempbn;

} Spake2p_Context;

static inline Spake2p_Context * to_inner_spake2p_context(Spake2pOpaqueContext * context)
{
    return SafePointerCast<Spake2p_Context *>(context);
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal(void)
{
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);
    memset(context, 0, sizeof(Spake2p_Context));

    M = &context->M;
    N = &context->N;
    X = &context->X;
    Y = &context->Y;
    L = &context->L;
    V = &context->V;
    Z = &context->Z;

    w0     = &context->w0;
    w1     = &context->w1;
    xy     = &context->xy;
    tempbn = &context->tempbn;

    return CHIP_NO_ERROR;
}

void Spake2p_P256_SHA256_HKDF_HMAC::Clear()
{
    VerifyOrReturn(state != CHIP_SPAKE2P_STATE::PREINIT);

    state = CHIP_SPAKE2P_STATE::PREINIT;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len,
                                              MutableByteSpan & out_span)
{
    HMAC_sha hmac;
    VerifyOrReturnError(out_span.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(hmac.HMAC_SHA256(key, key_len, in, in_len, out_span.data(), kSHA256_Hash_Length));
    out_span = out_span.SubSpan(0, kSHA256_Hash_Length);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::MacVerify(const uint8_t * key, size_t key_len, const uint8_t * mac, size_t mac_len,
                                                    const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    uint8_t computed_mac[kSHA256_Hash_Length];
    MutableByteSpan computed_mac_span{ computed_mac };
    VerifyOrExit(mac_len == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    SuccessOrExit(error = Mac(key, key_len, in, in_len, computed_mac_span));
    VerifyOrExit(computed_mac_span.size() == mac_len, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(IsBufferContentEqualConstantTime(mac, computed_mac, kSHA256_Hash_Length), error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const uint8_t * in, size_t in_len, void * fe)
{
    secEcp256Status_t result;
    uint32_t FE[SEC_ECP256_COORDINATE_WLEN];

    result = ECP256_ModularReductionN(FE, in, in_len);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    result = ECP256_FieldLoad((uint32_t *) fe, (const uint8_t *) FE, in_len);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, uint8_t * out, size_t out_len)
{
    secEcp256Status_t result;

    result = ECP256_FieldWrite(out, (uint8_t *) fe);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    secEcp256Status_t result;
    big_int256_t PrivateKey;

    result = ECP256_GeneratePrivateKey(&PrivateKey);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    result = ECP256_FieldWrite((uint8_t *) fe, (uint8_t *) &PrivateKey);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    secEcp256Status_t result;

    result = ECP256_ScalarMultiplicationModN((uint32_t *) fer, (const uint32_t *) fe1, (const uint32_t *) fe2);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const uint8_t * in, size_t in_len, void * R)
{
    ECP256_PointLoad((ecp256Point_t *) R, in, false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, uint8_t * out, size_t out_len)
{
    ECP256_PointWrite(out, (ecp256Point_t *) R, false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    secEcp256Status_t result;

    result = ECP256_PointMult((ecp256Point_t *) R, (const uint8_t *) P1, (const uint8_t *) fe1);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointAddMul(void * R, const void * P1, const void * fe1, const void * P2,
                                                      const void * fe2)
{
    secEcp256Status_t result;

    result = ECP256_DoublePointMulAdd(R, P1, fe1, P2, fe2);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointInvert(void * R)
{
    secEcp256Status_t result;

    result = ECP256_PointInvert((uint32_t *) R, (const uint32_t *) R);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointCofactorMul(void * R)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1sin, size_t w1sin_len)
{
    secEcp256Status_t result;
    ecp256Point_t gen_point;
    uint32_t W1[SEC_ECP256_COORDINATE_WLEN];

    result = ECP256_ModularReductionN(W1, w1sin, w1sin_len);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    result = ECP256_GeneratePublicKey((uint8_t *) &gen_point, (uint8_t *) &W1, NULL);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    ECP256_PointWrite(Lout, (ecp256Point_t *) &gen_point, false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointIsValid(void * R)
{
    VerifyOrReturnError(ECP256_PointValid((ecp256Point_t *) R), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
