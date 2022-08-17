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
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_SE05X_utils.h"

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

#if ENABLE_HSM_SPAKE_VERIFIER
const uint32_t w0in_id_v = 0x7D200001;
const uint32_t Lin_id_v  = 0x7D200002;
#endif

#if ENABLE_HSM_SPAKE_PROVER
const uint32_t w0in_id_p = 0x7D200003;
const uint32_t w1in_id_p = 0x7D200004;
#endif

void Spake2p_Finish_HSM(hsm_pake_context_t * phsm_pake_context)
{
#if ENABLE_REENTRANCY
    if (gex_sss_chip_ctx.ks.session != NULL)
    {
        Se05x_API_DeleteCryptoObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, phsm_pake_context->spake_objId);
        setObjID(phsm_pake_context->spake_objId, OBJ_ID_TABLE_OBJID_STATUS_FREE);

        if (spake_objects_created > 0)
        {
            spake_objects_created--;
        }
    }
#endif //#if ENABLE_REENTRANCY
    return;
}

CHIP_ERROR create_init_crypto_obj(chip::Crypto::CHIP_SPAKE2P_ROLE role, hsm_pake_context_t * phsm_pake_context)
{
    smStatus_t smstatus = SM_NOT_OK;
    SE05x_CryptoModeSubType_t subtype;

#if ENABLE_REENTRANCY
    SE05x_CryptoObjectID_t spakeObjectId = getObjID();
#else
    SE05x_CryptoObjectID_t spakeObjectId =
        (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? kSE05x_CryptoObject_PAKE_TYPE_B : kSE05x_CryptoObject_PAKE_TYPE_A;
    uint8_t list[1024] = {
        0,
    };
    size_t listlen            = sizeof(list);
    size_t i                  = 0;
    uint8_t create_crypto_obj = 1;
#endif

    ChipLogProgress(Crypto, "SE05x: Using Object Id --> %d", spakeObjectId);

    if (spakeObjectId != 0)
    {
        phsm_pake_context->spake_objId = spakeObjectId;
    }
    else
    {
        return CHIP_ERROR_INTERNAL;
    }

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    subtype.pakeMode = kSE05x_SPAKE2PLUS_P256_SHA256_HKDF_HMAC;

#if ENABLE_REENTRANCY
    VerifyOrReturnError(spake_objects_created < LIMIT_CRYPTO_OBJECTS, CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_CreateCryptoObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                            kSE05x_CryptoContext_PAKE, subtype);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);

    /* Increment number of crypto objects created */
    spake_objects_created++;

#else
    smstatus = Se05x_API_ReadCryptoObjectList(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, list, &listlen);
    for (i = 0; i < listlen; i += 4)
    {
        uint32_t cryptoObjectId = static_cast<uint32_t>(list[i + 1] | (list[i + 0] << 8));
        if (cryptoObjectId == spakeObjectId)
        {
            create_crypto_obj = 0;
        }
    }

    if (create_crypto_obj)
    {
        smstatus = Se05x_API_CreateCryptoObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                                kSE05x_CryptoContext_PAKE, subtype);
        VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_ComputeRoundOne_HSM(hsm_pake_context_t * phsm_pake_context, chip::Crypto::CHIP_SPAKE2P_ROLE role,
                                       const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    SE05x_CryptoObjectID_t spakeObjectId = phsm_pake_context->spake_objId;

    ChipLogProgress(Crypto, "SE05x: Using HSM for spake2p ComputeRoundOne");

    VerifyOrReturnError(out != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (pab_len > 0)
    {
        VerifyOrReturnError(pab != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        /* Need X/Y value to verify abort condition */
        VerifyOrReturnError(pab != NULL, CHIP_ERROR_INVALID_ARGUMENT);
    }

    smStatus_t smstatus = Se05x_API_PAKEComputeKeyShare(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                                        (uint8_t *) pab, pab_len, out, out_len);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_ComputeRoundTwo_HSM(hsm_pake_context_t * phsm_pake_context, chip::Crypto::CHIP_SPAKE2P_ROLE role,
                                       const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len, uint8_t * pKeyKe,
                                       size_t * pkeyKeLen)
{
    VerifyOrReturnError(in != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(pKeyKe != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(pkeyKeLen != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(Crypto, "SE05x: Using HSM for spake2p ComputeRoundTwo");

    const uint8_t * const pab = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? NULL : in;
    const size_t pab_len      = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? 0 : in_len;

    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    const SE05x_CryptoObjectID_t spakeObjectId = phsm_pake_context->spake_objId;
    const smStatus_t smstatus =
        Se05x_API_PAKEComputeSessionKeys(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                         (uint8_t *) pab, pab_len, pKeyKe, pkeyKeLen, out, out_len);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_KeyConfirm_HSM(hsm_pake_context_t * phsm_pake_context, chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * in,
                                  size_t in_len)
{
    VerifyOrReturnError(in != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    ChipLogProgress(Crypto, "SE05x: Using HSM for spake2p KeyConfirm");

    uint8_t presult                            = 0;
    const SE05x_CryptoObjectID_t spakeObjectId = phsm_pake_context->spake_objId;
    const smStatus_t smstatus = Se05x_API_PAKEVerifySessionKeys(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                                                spakeObjectId, (uint8_t *) in, in_len, &presult);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR_INTERNAL);

    return (presult == 1) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

namespace chip {
namespace Crypto {

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::Init(const uint8_t * context, size_t context_len)
{
    state = CHIP_SPAKE2P_STATE::PREINIT;

#if ENABLE_REENTRANCY
    static uint8_t alreadyInitialised = 0;
    if (alreadyInitialised == false)
    {
        delete_crypto_objects();
        init_cryptoObj_mutex();
        alreadyInitialised = true;
    }
#endif //#if ENABLE_REENTRANCY

    if (context_len > 0)
    {
        VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrReturnError(context_len <= sizeof(hsm_pake_context.spake_context), CHIP_ERROR_INTERNAL);
    memset(hsm_pake_context.spake_context, 0, sizeof(hsm_pake_context.spake_context));
    if (context_len > 0)
    {
        memcpy(hsm_pake_context.spake_context, context, context_len);
    }
    hsm_pake_context.spake_context_len = context_len;

    const CHIP_ERROR error = Spake2p::Init(context, context_len);
    if (error == CHIP_NO_ERROR)
    {
        state = CHIP_SPAKE2P_STATE::INIT;
    }

    return error;
}

#if ENABLE_HSM_SPAKE_VERIFIER
CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len,
                                                           const uint8_t * peer_identity, size_t peer_identity_len,
                                                           const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                                           size_t Lin_len)
{
    uint8_t w0in_mod[32] = {
        0,
    };
    size_t w0in_mod_len = 32;
    smStatus_t smstatus = SM_NOT_OK;

    VerifyOrReturnError(w0in != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(Lin != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::INIT, CHIP_ERROR_INTERNAL);
    if (my_identity_len > 0)
    {
        VerifyOrReturnError(my_identity != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (peer_identity_len > 0)
    {
        VerifyOrReturnError(peer_identity != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }

    ChipLogProgress(Crypto, "SE05x: HSM - BeginVerifier");

    ReturnErrorOnFailure(FELoad(w0in, w0in_len, w0));
    ReturnErrorOnFailure(FEWrite(w0, w0in_mod, w0in_mod_len));
    ReturnErrorOnFailure(create_init_crypto_obj(chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER, &hsm_pake_context));

    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                          SE05x_SPAKE2PLUS_DEVICE_TYPE_B);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR(chip::ChipError::Range::kPlatform, smstatus));

    ReturnErrorOnFailure(se05x_set_key(w0in_id_v, w0in_mod, w0in_mod_len, kSSS_KeyPart_Default, kSSS_CipherType_HMAC));
    ReturnErrorOnFailure(se05x_set_key(Lin_id_v, Lin, Lin_len, kSSS_KeyPart_Default, kSSS_CipherType_HMAC));

    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                        (uint8_t *) hsm_pake_context.spake_context, hsm_pake_context.spake_context_len,
                                        (uint8_t *) peer_identity, peer_identity_len, (uint8_t *) my_identity, my_identity_len);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR(chip::ChipError::Range::kPlatform, smstatus));

    smstatus = Se05x_API_PAKEInitCredentials(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                             hsm_pake_context.spake_objId, w0in_id_v, 0, Lin_id_v);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR(chip::ChipError::Range::kPlatform, smstatus));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::VERIFIER;

    return CHIP_NO_ERROR;
}
#endif

#if ENABLE_HSM_SPAKE_PROVER
CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::BeginProver(const uint8_t * my_identity, size_t my_identity_len,
                                                         const uint8_t * peer_identity, size_t peer_identity_len,
                                                         const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                                         size_t w1in_len)
{
    smStatus_t smstatus  = SM_NOT_OK;
    uint8_t w0in_mod[32] = {
        0,
    };
    size_t w0in_mod_len  = 32;
    uint8_t w1in_mod[32] = {
        0,
    };
    size_t w1in_mod_len = 32;

    VerifyOrReturnError(w0in != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(w1in != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (my_identity_len > 0)
    {
        VerifyOrReturnError(my_identity != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (peer_identity_len > 0)
    {
        VerifyOrReturnError(peer_identity != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::INIT, CHIP_ERROR_INTERNAL);

    ChipLogProgress(Crypto, "SE05x: HSM - BeginProver");

    ReturnErrorOnFailure(FELoad(w0in, w0in_len, w0));
    ReturnErrorOnFailure(FEWrite(w0, w0in_mod, w0in_mod_len));
    ReturnErrorOnFailure(FELoad(w1in, w1in_len, w1));
    ReturnErrorOnFailure(FEWrite(w1, w1in_mod, w1in_mod_len));
    ReturnErrorOnFailure(create_init_crypto_obj(chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER, &hsm_pake_context));

    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                          SE05x_SPAKE2PLUS_DEVICE_TYPE_A);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR(chip::ChipError::Range::kPlatform, smstatus));

    ReturnErrorOnFailure(se05x_set_key(w0in_id_p, w0in_mod, w0in_mod_len, kSSS_KeyPart_Default, kSSS_CipherType_HMAC));
    ReturnErrorOnFailure(se05x_set_key(w1in_id_p, w1in_mod, w1in_mod_len, kSSS_KeyPart_Default, kSSS_CipherType_HMAC));

    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                        (uint8_t *) hsm_pake_context.spake_context, hsm_pake_context.spake_context_len,
                                        (uint8_t *) my_identity, my_identity_len, (uint8_t *) peer_identity, peer_identity_len);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR(chip::ChipError::Range::kPlatform, smstatus));

    smstatus = Se05x_API_PAKEInitCredentials(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                             hsm_pake_context.spake_objId, w0in_id_p, w1in_id_p, 0);
    VerifyOrReturnError(smstatus == SM_OK, CHIP_ERROR(chip::ChipError::Range::kPlatform, smstatus));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::PROVER;

    return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::STARTED, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(*out_len >= point_size, CHIP_ERROR_INTERNAL);

#if !ENABLE_HSM_SPAKE_VERIFIER
    const bool sw_rollback_verifier = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER);
#else
    constexpr bool sw_rollback_verifier = false;
#endif

#if ((CHIP_CRYPTO_HSM) && (!ENABLE_HSM_SPAKE_PROVER))
    const bool sw_rollback_prover = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER);
#else
    constexpr bool sw_rollback_prover   = false;
#endif

    if (sw_rollback_verifier || sw_rollback_prover)
    {
        return Spake2p::ComputeRoundOne(pab, pab_len, out, out_len);
    }

    CHIP_ERROR error = Spake2p_ComputeRoundOne_HSM(&hsm_pake_context, role, pab, pab_len, out, out_len);
    if (CHIP_NO_ERROR == error)
    {
        state = CHIP_SPAKE2P_STATE::R1;
    }
    return error;
}

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len)
{
    VerifyOrReturnError(*out_len >= hash_size, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::R1, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(in_len == point_size, CHIP_ERROR_INTERNAL);

#if !ENABLE_HSM_SPAKE_VERIFIER
    const bool sw_rollback_verifier = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER);
#else
    constexpr bool sw_rollback_verifier = false;
#endif

#if !ENABLE_HSM_SPAKE_PROVER
    const bool sw_rollback_prover = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER);
#else
    constexpr bool sw_rollback_prover   = false;
#endif

    if (sw_rollback_verifier || sw_rollback_prover)
    {
        return Spake2p::ComputeRoundTwo(in, in_len, out, out_len);
    }

    uint8_t pKeyKe[16] = {
        0,
    };
    size_t pkeyKeLen = sizeof(pKeyKe);

    const CHIP_ERROR error = Spake2p_ComputeRoundTwo_HSM(&hsm_pake_context, role, in, in_len, out, out_len, pKeyKe, &pkeyKeLen);
    if (CHIP_NO_ERROR == error)
    {
        memcpy((Kae + 16), pKeyKe, pkeyKeLen);
        state = CHIP_SPAKE2P_STATE::R2;
    }
    return error;
}

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::KeyConfirm(const uint8_t * in, size_t in_len)
{
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::R2, CHIP_ERROR_INTERNAL);

#if !ENABLE_HSM_SPAKE_VERIFIER
    const bool sw_rollback_verifier = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER);
#else
    constexpr bool sw_rollback_verifier = false;
#endif

#if !ENABLE_HSM_SPAKE_PROVER
    const bool sw_rollback_prover = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER);
#else
    constexpr bool sw_rollback_prover   = false;
#endif

    if (sw_rollback_verifier || sw_rollback_prover)
    {
        return Spake2p::KeyConfirm(in, in_len);
    }
    const CHIP_ERROR error = Spake2p_KeyConfirm_HSM(&hsm_pake_context, role, in, in_len);
    if (CHIP_NO_ERROR == error)
    {
        state = CHIP_SPAKE2P_STATE::KC;
    }

    Spake2p_Finish_HSM(&hsm_pake_context);

    return error;
}

} // namespace Crypto
} // namespace chip

#endif //#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))
