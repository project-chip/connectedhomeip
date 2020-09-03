/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Platform agnostic implementation of CHIP crypto algorithms
 */

#include "CHIPCryptoPAL.h"
#include <string.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Crypto {

CHIP_ERROR Spake2p::InternalHash(const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    uint64_t u64_len = in_len;

    uint8_t lb[8];
    lb[0] = (u64_len >> 0) & 0xff;
    lb[1] = (u64_len >> 8) & 0xff;
    lb[2] = (u64_len >> 16) & 0xff;
    lb[3] = (u64_len >> 24) & 0xff;
    lb[4] = (u64_len >> 32) & 0xff;
    lb[5] = (u64_len >> 40) & 0xff;
    lb[6] = (u64_len >> 48) & 0xff;
    lb[7] = (u64_len >> 56) & 0xff;

    error = Hash(lb, sizeof(lb));
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    if (in != NULL)
    {
        error = Hash(in, in_len);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    return error;
}

Spake2p::Spake2p(size_t _fe_size, size_t _point_size, size_t _hash_size)
{
    fe_size    = _fe_size;
    point_size = _point_size;
    hash_size  = _hash_size;

    Kca = &Kcab[0];
    Kcb = &Kcab[hash_size / 2];
    Ka  = &Kae[0];
    Ke  = &Kae[hash_size / 2];

    M  = nullptr;
    N  = nullptr;
    G  = nullptr;
    X  = nullptr;
    Y  = nullptr;
    L  = nullptr;
    Z  = nullptr;
    V  = nullptr;
    w0 = nullptr;
    w1 = nullptr;
    xy = nullptr;

    order  = nullptr;
    tempbn = nullptr;
}

CHIP_ERROR Spake2p::Init(const uint8_t * context, size_t context_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    state            = CHIP_SPAKE2P_STATE::PREINIT;

    error = InitImpl();
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointLoad(spake2p_M_p256, sizeof(spake2p_M_p256), M);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointLoad(spake2p_N_p256, sizeof(spake2p_N_p256), N);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = InternalHash(context, context_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::INIT;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::WriteMN(void)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    error = InternalHash(spake2p_M_p256, sizeof(spake2p_M_p256));
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    error = InternalHash(spake2p_N_p256, sizeof(spake2p_N_p256));
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                  size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                  size_t Lin_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::INIT, error = CHIP_ERROR_INTERNAL);

    error = InternalHash(peer_identity, peer_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = InternalHash(my_identity, my_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = WriteMN();
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FELoad(w0in, w0in_len, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointLoad(Lin, Lin_len, L);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::VERIFIER;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                size_t w1in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::INIT, error = CHIP_ERROR_INTERNAL);

    error = InternalHash(my_identity, my_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = InternalHash(peer_identity, peer_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = WriteMN();
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FELoad(w0in, w0in_len, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FELoad(w1in, w1in_len, w1);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::PROVER;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::ComputeRoundOne(uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    void * MN        = NULL; // Choose M if a prover, N if a verifier
    void * XY        = NULL; // Choose X if a prover, Y if a verifier

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::STARTED, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(*out_len >= point_size, error = CHIP_ERROR_INTERNAL);

    error = FEGenerate(xy);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        MN = M;
        XY = X;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        MN = N;
        XY = Y;
    }
    VerifyOrExit(MN != NULL, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(XY != NULL, error = CHIP_ERROR_INTERNAL);

    error = PointAddMul(XY, G, xy, MN, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointWrite(XY, out, *out_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::R1;
    error = CHIP_NO_ERROR;
exit:
    *out_len = point_size;
    return error;
}

CHIP_ERROR Spake2p::ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    uint8_t point_buffer[kMAX_Point_Length];
    void * MN        = NULL; // Choose N if a prover, M if a verifier
    void * XY        = NULL; // Choose Y if a prover, X if a verifier
    uint8_t * Kcaorb = NULL; // Choose Kca if a prover, Kcb if a verifier

    VerifyOrExit(*out_len >= hash_size, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(in_len == point_size, error = CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        error = PointWrite(X, point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

        error = InternalHash(point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

        error = InternalHash(in, in_len);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

        MN     = N;
        XY     = Y;
        Kcaorb = Kca;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        error = InternalHash(in, in_len);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

        error = PointWrite(Y, point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

        error = InternalHash(point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

        MN     = M;
        XY     = X;
        Kcaorb = Kcb;
    }
    VerifyOrExit(MN != NULL, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(XY != NULL, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(Kcab != NULL, error = CHIP_ERROR_INTERNAL);

    error = PointLoad(in, in_len, XY);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    error = PointIsValid(XY);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FEMul(tempbn, xy, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointInvert(MN);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointAddMul(Z, XY, xy, MN, tempbn);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointCofactorMul(Z);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        error = FEMul(tempbn, w1, w0);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
        error = PointAddMul(V, XY, w1, MN, tempbn);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        error = PointMul(V, L, xy);
        VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    }

    error = PointCofactorMul(V);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointWrite(Z, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    error = InternalHash(point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointWrite(V, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    error = InternalHash(point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FEWrite(w0, point_buffer, fe_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    error = InternalHash(point_buffer, fe_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = GenerateKeys();
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = Mac(Kcaorb, hash_size / 2, in, in_len, out);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::R2;
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size;
    return error;
}

CHIP_ERROR Spake2p::GenerateKeys(void)
{
    CHIP_ERROR error                         = CHIP_ERROR_INTERNAL;
    static const uint8_t info_keyconfirm[16] = { 'C', 'o', 'n', 'f', 'i', 'r', 'm', 'a', 't', 'i', 'o', 'n', 'K', 'e', 'y', 's' };

    error = HashFinalize(Kae);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = KDF(Ka, hash_size / 2, NULL, 0, info_keyconfirm, sizeof(info_keyconfirm), Kcab, hash_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::KeyConfirm(const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    uint8_t point_buffer[kP256_Point_Length];
    void * XY        = NULL; // Choose X if a prover, Y if a verifier
    uint8_t * Kcaorb = NULL; // Choose Kcb if a prover, Kca if a verifier

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R2, error = CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        XY     = X;
        Kcaorb = Kcb;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        XY     = Y;
        Kcaorb = Kca;
    }
    VerifyOrExit(XY != NULL, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(Kcaorb != NULL, error = CHIP_ERROR_INTERNAL);

    error = PointWrite(XY, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = MacVerify(Kcaorb, hash_size / 2, in, in_len, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::KC;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::GetKeys(uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::KC, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(*out_len >= hash_size / 2, error = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(out, Ke, hash_size / 2);
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size / 2;
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitImpl(void)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    error = sha256_hash_ctx.Begin();
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = InitInternal();
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Hash(const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    error = sha256_hash_ctx.AddData(in, in_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::HashFinalize(uint8_t * out)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    error = sha256_hash_ctx.Finish(out);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::KDF(const uint8_t * ikm, const size_t ikm_len, const uint8_t * salt,
                                              const size_t salt_len, const uint8_t * info, const size_t info_len, uint8_t * out,
                                              size_t out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    error = HKDF_SHA256(ikm, ikm_len, salt, salt_len, info, info_len, out, out_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

} // namespace Crypto
} // namespace chip
