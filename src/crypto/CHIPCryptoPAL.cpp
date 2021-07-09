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

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

CHIP_ERROR Spake2p::InternalHash(const uint8_t * in, size_t in_len)
{
    const uint64_t u64_len = in_len;

    uint8_t lb[8];
    lb[0] = static_cast<uint8_t>((u64_len >> 0) & 0xff);
    lb[1] = static_cast<uint8_t>((u64_len >> 8) & 0xff);
    lb[2] = static_cast<uint8_t>((u64_len >> 16) & 0xff);
    lb[3] = static_cast<uint8_t>((u64_len >> 24) & 0xff);
    lb[4] = static_cast<uint8_t>((u64_len >> 32) & 0xff);
    lb[5] = static_cast<uint8_t>((u64_len >> 40) & 0xff);
    lb[6] = static_cast<uint8_t>((u64_len >> 48) & 0xff);
    lb[7] = static_cast<uint8_t>((u64_len >> 56) & 0xff);

    ReturnErrorOnFailure(Hash(lb, sizeof(lb)));
    if (in != nullptr)
    {
        ReturnErrorOnFailure(Hash(in, in_len));
    }

    return CHIP_NO_ERROR;
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
    state = CHIP_SPAKE2P_STATE::PREINIT;

    ReturnErrorOnFailure(InitImpl());
    ReturnErrorOnFailure(PointLoad(spake2p_M_p256, sizeof(spake2p_M_p256), M));
    ReturnErrorOnFailure(PointLoad(spake2p_N_p256, sizeof(spake2p_N_p256), N));
    ReturnErrorOnFailure(InternalHash(context, context_len));

    state = CHIP_SPAKE2P_STATE::INIT;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::WriteMN()
{
    ReturnErrorOnFailure(InternalHash(spake2p_M_p256, sizeof(spake2p_M_p256)));
    ReturnErrorOnFailure(InternalHash(spake2p_N_p256, sizeof(spake2p_N_p256)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                  size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                  size_t Lin_len)
{
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::INIT, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(InternalHash(peer_identity, peer_identity_len));
    ReturnErrorOnFailure(InternalHash(my_identity, my_identity_len));
    ReturnErrorOnFailure(WriteMN());
    ReturnErrorOnFailure(FELoad(w0in, w0in_len, w0));
    ReturnErrorOnFailure(PointLoad(Lin, Lin_len, L));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::VERIFIER;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                size_t w1in_len)
{
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::INIT, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(InternalHash(my_identity, my_identity_len));
    ReturnErrorOnFailure(InternalHash(peer_identity, peer_identity_len));
    ReturnErrorOnFailure(WriteMN());
    ReturnErrorOnFailure(FELoad(w0in, w0in_len, w0));
    ReturnErrorOnFailure(FELoad(w1in, w1in_len, w1));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::PROVER;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    void * MN        = nullptr; // Choose M if a prover, N if a verifier
    void * XY        = nullptr; // Choose X if a prover, Y if a verifier

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::STARTED, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(*out_len >= point_size, error = CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(FEGenerate(xy));

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
    VerifyOrExit(MN != nullptr, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(XY != nullptr, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = PointAddMul(XY, G, xy, MN, w0));
    SuccessOrExit(error = PointWrite(XY, out, *out_len));

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
    void * MN        = nullptr; // Choose N if a prover, M if a verifier
    void * XY        = nullptr; // Choose Y if a prover, X if a verifier
    uint8_t * Kcaorb = nullptr; // Choose Kca if a prover, Kcb if a verifier

    VerifyOrExit(*out_len >= hash_size, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(in_len == point_size, error = CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        SuccessOrExit(error = PointWrite(X, point_buffer, point_size));
        SuccessOrExit(error = InternalHash(point_buffer, point_size));
        SuccessOrExit(error = InternalHash(in, in_len));

        MN     = N;
        XY     = Y;
        Kcaorb = Kca;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        SuccessOrExit(error = InternalHash(in, in_len));
        SuccessOrExit(error = PointWrite(Y, point_buffer, point_size));
        SuccessOrExit(error = InternalHash(point_buffer, point_size));

        MN     = M;
        XY     = X;
        Kcaorb = Kcb;
    }
    VerifyOrExit(MN != nullptr, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(XY != nullptr, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = PointLoad(in, in_len, XY));
    SuccessOrExit(error = PointIsValid(XY));
    SuccessOrExit(error = FEMul(tempbn, xy, w0));
    SuccessOrExit(error = PointInvert(MN));
    SuccessOrExit(error = PointAddMul(Z, XY, xy, MN, tempbn));
    SuccessOrExit(error = PointCofactorMul(Z));

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        SuccessOrExit(error = FEMul(tempbn, w1, w0));
        SuccessOrExit(error = PointAddMul(V, XY, w1, MN, tempbn));
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        SuccessOrExit(error = PointMul(V, L, xy));
    }

    SuccessOrExit(error = PointCofactorMul(V));
    SuccessOrExit(error = PointWrite(Z, point_buffer, point_size));
    SuccessOrExit(error = InternalHash(point_buffer, point_size));

    SuccessOrExit(error = PointWrite(V, point_buffer, point_size));
    SuccessOrExit(error = InternalHash(point_buffer, point_size));

    SuccessOrExit(error = FEWrite(w0, point_buffer, fe_size));
    SuccessOrExit(error = InternalHash(point_buffer, fe_size));

    SuccessOrExit(error = GenerateKeys());

    SuccessOrExit(error = Mac(Kcaorb, hash_size / 2, in, in_len, out));

    state = CHIP_SPAKE2P_STATE::R2;
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size;
    return error;
}

CHIP_ERROR Spake2p::GenerateKeys()
{
    static const uint8_t info_keyconfirm[16] = { 'C', 'o', 'n', 'f', 'i', 'r', 'm', 'a', 't', 'i', 'o', 'n', 'K', 'e', 'y', 's' };

    ReturnErrorOnFailure(HashFinalize(Kae));
    ReturnErrorOnFailure(KDF(Ka, hash_size / 2, nullptr, 0, info_keyconfirm, sizeof(info_keyconfirm), Kcab, hash_size));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::KeyConfirm(const uint8_t * in, size_t in_len)
{
    uint8_t point_buffer[kP256_Point_Length];
    void * XY        = nullptr; // Choose X if a prover, Y if a verifier
    uint8_t * Kcaorb = nullptr; // Choose Kcb if a prover, Kca if a verifier

    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::R2, CHIP_ERROR_INTERNAL);

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
    VerifyOrReturnError(XY != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(Kcaorb != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(PointWrite(XY, point_buffer, point_size));
    ReturnErrorOnFailure(MacVerify(Kcaorb, hash_size / 2, in, in_len, point_buffer, point_size));

    state = CHIP_SPAKE2P_STATE::KC;
    return CHIP_NO_ERROR;
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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitImpl()
{
    ReturnErrorOnFailure(sha256_hash_ctx.Begin());
    ReturnErrorOnFailure(InitInternal());
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Hash(const uint8_t * in, size_t in_len)
{
    ReturnErrorOnFailure(sha256_hash_ctx.AddData(in, in_len));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::HashFinalize(uint8_t * out)
{
    ReturnErrorOnFailure(sha256_hash_ctx.Finish(out));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::KDF(const uint8_t * ikm, const size_t ikm_len, const uint8_t * salt,
                                              const size_t salt_len, const uint8_t * info, const size_t info_len, uint8_t * out,
                                              size_t out_len)
{
    HKDF_sha_crypto mHKDF;

    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(ikm, ikm_len, salt, salt_len, info, info_len, out, out_len));

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
