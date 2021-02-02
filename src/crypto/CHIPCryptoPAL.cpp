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
    CHIP_ERROR error = _CHIP_ERROR(512);
    uint64_t u64_len = in_len;

    uint8_t lb[8];
    lb[0] = static_cast<uint8_t>((u64_len >> 0) & 0xff);
    lb[1] = static_cast<uint8_t>((u64_len >> 8) & 0xff);
    lb[2] = static_cast<uint8_t>((u64_len >> 16) & 0xff);
    lb[3] = static_cast<uint8_t>((u64_len >> 24) & 0xff);
    lb[4] = static_cast<uint8_t>((u64_len >> 32) & 0xff);
    lb[5] = static_cast<uint8_t>((u64_len >> 40) & 0xff);
    lb[6] = static_cast<uint8_t>((u64_len >> 48) & 0xff);
    lb[7] = static_cast<uint8_t>((u64_len >> 56) & 0xff);

    error = Hash(lb, sizeof(lb));
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(513));
    if (in != nullptr)
    {
        error = Hash(in, in_len);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(514));
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
    CHIP_ERROR error = _CHIP_ERROR(515);
    state            = CHIP_SPAKE2P_STATE::PREINIT;

    error = InitImpl();
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(516));

    error = PointLoad(spake2p_M_p256, sizeof(spake2p_M_p256), M);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(517));

    error = PointLoad(spake2p_N_p256, sizeof(spake2p_N_p256), N);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(518));

    error = InternalHash(context, context_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(519));

    state = CHIP_SPAKE2P_STATE::INIT;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::WriteMN()
{
    CHIP_ERROR error = _CHIP_ERROR(520);

    error = InternalHash(spake2p_M_p256, sizeof(spake2p_M_p256));
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(521));
    error = InternalHash(spake2p_N_p256, sizeof(spake2p_N_p256));
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(522));

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                  size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                  size_t Lin_len)
{
    CHIP_ERROR error = _CHIP_ERROR(523);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::INIT, error = _CHIP_ERROR(524));

    error = InternalHash(peer_identity, peer_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(525));

    error = InternalHash(my_identity, my_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(526));

    error = WriteMN();
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(527));

    error = FELoad(w0in, w0in_len, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(528));

    error = PointLoad(Lin, Lin_len, L);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(529));

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
    CHIP_ERROR error = _CHIP_ERROR(530);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::INIT, error = _CHIP_ERROR(531));

    error = InternalHash(my_identity, my_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(532));

    error = InternalHash(peer_identity, peer_identity_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(533));

    error = WriteMN();
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(534));

    error = FELoad(w0in, w0in_len, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(535));

    error = FELoad(w1in, w1in_len, w1);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(536));

    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(537));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::PROVER;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::ComputeRoundOne(uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = _CHIP_ERROR(538);
    void * MN        = nullptr; // Choose M if a prover, N if a verifier
    void * XY        = nullptr; // Choose X if a prover, Y if a verifier

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::STARTED, error = _CHIP_ERROR(539));
    VerifyOrExit(*out_len >= point_size, error = _CHIP_ERROR(540));

    error = FEGenerate(xy);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(541));

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
    VerifyOrExit(MN != nullptr, error = _CHIP_ERROR(542));
    VerifyOrExit(XY != nullptr, error = _CHIP_ERROR(543));

    error = PointAddMul(XY, G, xy, MN, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(544));

    error = PointWrite(XY, out, *out_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(545));

    state = CHIP_SPAKE2P_STATE::R1;
    error = CHIP_NO_ERROR;
exit:
    *out_len = point_size;
    return error;
}

CHIP_ERROR Spake2p::ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = _CHIP_ERROR(546);
    uint8_t point_buffer[kMAX_Point_Length];
    void * MN        = nullptr; // Choose N if a prover, M if a verifier
    void * XY        = nullptr; // Choose Y if a prover, X if a verifier
    uint8_t * Kcaorb = nullptr; // Choose Kca if a prover, Kcb if a verifier

    VerifyOrExit(*out_len >= hash_size, error = _CHIP_ERROR(547));
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R1, error = _CHIP_ERROR(548));
    VerifyOrExit(in_len == point_size, error = _CHIP_ERROR(549));

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        error = PointWrite(X, point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(550));

        error = InternalHash(point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(551));

        error = InternalHash(in, in_len);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(552));

        MN     = N;
        XY     = Y;
        Kcaorb = Kca;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        error = InternalHash(in, in_len);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(553));

        error = PointWrite(Y, point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(554));

        error = InternalHash(point_buffer, point_size);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(555));

        MN     = M;
        XY     = X;
        Kcaorb = Kcb;
    }
    VerifyOrExit(MN != nullptr, error = _CHIP_ERROR(556));
    VerifyOrExit(XY != nullptr, error = _CHIP_ERROR(557));

    error = PointLoad(in, in_len, XY);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(558));
    error = PointIsValid(XY);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(559));

    error = FEMul(tempbn, xy, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(560));

    error = PointInvert(MN);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(561));

    error = PointAddMul(Z, XY, xy, MN, tempbn);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(562));

    error = PointCofactorMul(Z);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(563));

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        error = FEMul(tempbn, w1, w0);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(564));
        error = PointAddMul(V, XY, w1, MN, tempbn);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(565));
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        error = PointMul(V, L, xy);
        VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(566));
    }

    error = PointCofactorMul(V);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(567));

    error = PointWrite(Z, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(568));
    error = InternalHash(point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(569));

    error = PointWrite(V, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(570));
    error = InternalHash(point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(571));

    error = FEWrite(w0, point_buffer, fe_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(572));
    error = InternalHash(point_buffer, fe_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(573));

    error = GenerateKeys();
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(574));

    error = Mac(Kcaorb, hash_size / 2, in, in_len, out);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(575));

    state = CHIP_SPAKE2P_STATE::R2;
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size;
    return error;
}

CHIP_ERROR Spake2p::GenerateKeys()
{
    CHIP_ERROR error                         = _CHIP_ERROR(576);
    static const uint8_t info_keyconfirm[16] = { 'C', 'o', 'n', 'f', 'i', 'r', 'm', 'a', 't', 'i', 'o', 'n', 'K', 'e', 'y', 's' };

    error = HashFinalize(Kae);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(577));

    error = KDF(Ka, hash_size / 2, nullptr, 0, info_keyconfirm, sizeof(info_keyconfirm), Kcab, hash_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(578));

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::KeyConfirm(const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = _CHIP_ERROR(579);
    uint8_t point_buffer[kP256_Point_Length];
    void * XY        = nullptr; // Choose X if a prover, Y if a verifier
    uint8_t * Kcaorb = nullptr; // Choose Kcb if a prover, Kca if a verifier

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R2, error = _CHIP_ERROR(580));

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
    VerifyOrExit(XY != nullptr, error = _CHIP_ERROR(581));
    VerifyOrExit(Kcaorb != nullptr, error = _CHIP_ERROR(582));

    error = PointWrite(XY, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(583));

    error = MacVerify(Kcaorb, hash_size / 2, in, in_len, point_buffer, point_size);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(584));

    state = CHIP_SPAKE2P_STATE::KC;
    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p::GetKeys(uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = _CHIP_ERROR(585);

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::KC, error = _CHIP_ERROR(586));
    VerifyOrExit(*out_len >= hash_size / 2, error = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(out, Ke, hash_size / 2);
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size / 2;
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitImpl()
{
    CHIP_ERROR error = _CHIP_ERROR(587);

    error = sha256_hash_ctx.Begin();
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(588));

    error = InitInternal();
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(589));

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Hash(const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = _CHIP_ERROR(590);

    error = sha256_hash_ctx.AddData(in, in_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(591));

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::HashFinalize(uint8_t * out)
{
    CHIP_ERROR error = _CHIP_ERROR(592);

    error = sha256_hash_ctx.Finish(out);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(593));

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::KDF(const uint8_t * ikm, const size_t ikm_len, const uint8_t * salt,
                                              const size_t salt_len, const uint8_t * info, const size_t info_len, uint8_t * out,
                                              size_t out_len)
{
    CHIP_ERROR error = _CHIP_ERROR(594);

    error = HKDF_SHA256(ikm, ikm_len, salt, salt_len, info, info_len, out, out_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = _CHIP_ERROR(595));

    error = CHIP_NO_ERROR;
exit:
    return error;
}

} // namespace Crypto
} // namespace chip
