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

const int m_id = 0x2345;
const int n_id = 0x2346;

#if ENABLE_HSM_SPAKE_VERIFIER
const int w0in_id_v = 0x2347;
const int Lin_id_v  = 0x2348;
#endif

#if ENABLE_HSM_SPAKE_PROVER
const int w0in_id_p = 0x2349;
const int w1in_id_p = 0x2350;
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
    CHIP_ERROR error    = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;
    SE05x_CryptoModeSubType_t subtype;

#if ENABLE_REENTRANCY
    SE05x_CryptoObjectID_t spakeObjectId = getObjID();
#else
    SE05x_CryptoObjectID_t spakeObjectId =
        (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? kSE05x_CryptoObject_SPAKE_VERIFIER : kSE05x_CryptoObject_SPAKE_PROVER;
    uint8_t list[1024] = {
        0,
    };
    size_t listlen            = sizeof(list);
    size_t i                  = 0;
    uint8_t create_crypto_obj = 1;
#endif

    ChipLogProgress(Crypto, "Using Object Id --> %d \n", spakeObjectId);

    if (spakeObjectId != 0)
    {
        phsm_pake_context->spake_objId = spakeObjectId;
    }
    else
    {
        return CHIP_ERROR_INTERNAL;
    }

    se05x_sessionOpen();

    se05x_delete_key(m_id);
    error = se05x_set_key(m_id, chip::Crypto::spake2p_M_p256, sizeof(chip::Crypto::spake2p_M_p256), kSSS_KeyPart_Public,
                          kSSS_CipherType_EC_NIST_P);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    se05x_delete_key(n_id);
    error = se05x_set_key(n_id, chip::Crypto::spake2p_N_p256, sizeof(chip::Crypto::spake2p_N_p256), kSSS_KeyPart_Public,
                          kSSS_CipherType_EC_NIST_P);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    subtype.spakeAlgo = kSE05x_SpakeAlgo_P256_SHA256_HKDF_HMAC;

#if ENABLE_REENTRANCY
    VerifyOrExit(spake_objects_created < LIMIT_CRYPTO_OBJECTS, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_CreateCryptoObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                            kSE05x_CryptoContext_SPAKE, subtype);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    /* Increment number of crypto objects created */
    spake_objects_created++;

#else
    smstatus = Se05x_API_ReadCryptoObjectList(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, list, &listlen);
    for (i = 0; i < listlen; i += 4)
    {
        uint32_t cryptoObjectId = list[i + 1] | (list[i + 0] << 8);
        if (cryptoObjectId == spakeObjectId)
        {
            create_crypto_obj = 0;
        }
    }

    if (create_crypto_obj)
    {
        smstatus = Se05x_API_CreateCryptoObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                                kSE05x_CryptoContext_SPAKE, subtype);
        VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
    }
#endif

    smstatus = Se05x_API_PAKEInitProtocol(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId, m_id, n_id);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_ComputeRoundOne_HSM(hsm_pake_context_t * phsm_pake_context, chip::Crypto::CHIP_SPAKE2P_ROLE role,
                                       const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error    = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;

#if SSS_HAVE_SE05X_VER_GTE_16_03
#else
    uint8_t * prand     = NULL;
    size_t prand_len    = 0;
    uint8_t tempBuf[32] = {
        0,
    };
#endif
    SE05x_CryptoObjectID_t spakeObjectId = phsm_pake_context->spake_objId;

    VerifyOrExit(out != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_len != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (pab_len > 0)
    {
        VerifyOrExit(pab != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        if (pab == NULL)
        {
            /* Need X/Y value to verify abort condition */
            goto exit;
        }
    }

#if SSS_HAVE_SE05X_VER_GTE_16_03
#else
    /* Generate random numbers from SE */
    sss_status_t status = kStatus_SSS_Success;
    sss_rng_context_t rng_ctx;

    status = sss_rng_context_init(&rng_ctx, &gex_sss_chip_ctx.session);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_rng_get_random(&rng_ctx, tempBuf, sizeof(tempBuf));
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    sss_rng_context_free(&rng_ctx);

    prand     = tempBuf;
    prand_len = sizeof(tempBuf);

#endif

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_03
    smstatus = Se05x_API_PAKEComputeKeyShare(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                             (uint8_t *) pab, pab_len, out, out_len);
#else
    smstatus  = Se05x_API_PAKEComputeKeyShare(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                             (uint8_t *) pab, pab_len, out, out_len, prand, prand_len);
#endif
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_ComputeRoundTwo_HSM(hsm_pake_context_t * phsm_pake_context, chip::Crypto::CHIP_SPAKE2P_ROLE role,
                                       const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len, uint8_t * pKeyKe,
                                       size_t * pkeyKeLen)
{
    CHIP_ERROR error                     = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus                  = SM_NOT_OK;
    const uint8_t * pab                  = NULL;
    size_t pab_len                       = 0;
    SE05x_CryptoObjectID_t spakeObjectId = phsm_pake_context->spake_objId;

    VerifyOrExit(in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_len != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pKeyKe != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pkeyKeLen != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    pab     = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? NULL : in;
    pab_len = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? 0 : in_len;

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEComputeSessionKeys(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                                (uint8_t *) pab, pab_len, pKeyKe, pkeyKeLen, out, out_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_KeyConfirm_HSM(hsm_pake_context_t * phsm_pake_context, chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * in,
                                  size_t in_len)
{
    CHIP_ERROR error                     = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus                  = SM_NOT_OK;
    uint8_t presult                      = 0;
    SE05x_CryptoObjectID_t spakeObjectId = phsm_pake_context->spake_objId;

    VerifyOrExit(in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEVerifySessionKeys(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, spakeObjectId,
                                               (uint8_t *) in, in_len, &presult);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = (presult == 1) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
exit:
    return error;
}

namespace chip {
namespace Crypto {

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::Init(const uint8_t * context, size_t context_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    state            = CHIP_SPAKE2P_STATE::PREINIT;

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
        VerifyOrExit(context != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrExit(context_len <= sizeof(hsm_pake_context.spake_context), error = CHIP_ERROR_INTERNAL);
    memset(hsm_pake_context.spake_context, 0, sizeof(hsm_pake_context.spake_context));
    memcpy(hsm_pake_context.spake_context, context, context_len);
    hsm_pake_context.spake_context_len = context_len;

    error = Spake2p::Init(context, context_len);
    if (error == CHIP_NO_ERROR)
    {
        state = CHIP_SPAKE2P_STATE::INIT;
    }

exit:
    return error;
}

#if ENABLE_HSM_SPAKE_VERIFIER
CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len,
                                                           const uint8_t * peer_identity, size_t peer_identity_len,
                                                           const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                                           size_t Lin_len)
{
    CHIP_ERROR error     = CHIP_ERROR_INTERNAL;
    uint8_t w0in_mod[32] = {
        0,
    };
    size_t w0in_mod_len = 32;
    smStatus_t smstatus = SM_NOT_OK;

    VerifyOrExit(w0in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(Lin != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::INIT, error = CHIP_ERROR_INTERNAL);
    if (my_identity_len > 0)
    {
        VerifyOrExit(my_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (peer_identity_len > 0)
    {
        VerifyOrExit(peer_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    error = FELoad(w0in, w0in_len, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FEWrite(w0, w0in_mod, w0in_mod_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = create_init_crypto_obj(chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER, &hsm_pake_context);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, SE05x_SPAKEDevice_B,
                                          hsm_pake_context.spake_objId);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                        (uint8_t *) hsm_pake_context.spake_context, hsm_pake_context.spake_context_len,
                                        (uint8_t *) peer_identity, peer_identity_len, (uint8_t *) my_identity, my_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#else
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, SE05x_SPAKEDevice_B,
                                          hsm_pake_context.spake_objId, (uint8_t *) hsm_pake_context.spake_context,
                                          hsm_pake_context.spake_context_len, (uint8_t *) peer_identity, peer_identity_len,
                                          (uint8_t *) my_identity, my_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#endif

    error = se05x_set_key(w0in_id_v, w0in_mod, w0in_mod_len, kSSS_KeyPart_Default, kSSS_CipherType_AES);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = se05x_set_key(Lin_id_v, Lin, Lin_len, kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEInitCredentials(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                             hsm_pake_context.spake_objId, w0in_id_v, 0, Lin_id_v);
#else
    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                        w0in_id_v, 0, Lin_id_v);
#endif
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::VERIFIER;
    error = CHIP_NO_ERROR;

exit:
    return error;
}
#endif

#if ENABLE_HSM_SPAKE_PROVER
CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::BeginProver(const uint8_t * my_identity, size_t my_identity_len,
                                                         const uint8_t * peer_identity, size_t peer_identity_len,
                                                         const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                                         size_t w1in_len)
{
    CHIP_ERROR error     = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus  = SM_NOT_OK;
    uint8_t w0in_mod[32] = {
        0,
    };
    size_t w0in_mod_len  = 32;
    uint8_t w1in_mod[32] = {
        0,
    };
    size_t w1in_mod_len = 32;

    VerifyOrExit(w0in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(w1in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (my_identity_len > 0)
    {
        VerifyOrExit(my_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (peer_identity_len > 0)
    {
        VerifyOrExit(peer_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::INIT, error = CHIP_ERROR_INTERNAL);

    error = FELoad(w0in, w0in_len, w0);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FEWrite(w0, w0in_mod, w0in_mod_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FELoad(w1in, w1in_len, w1);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = FEWrite(w1, w1in_mod, w1in_mod_len);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = create_init_crypto_obj(chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER, &hsm_pake_context);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, SE05x_SPAKEDevice_A,
                                          hsm_pake_context.spake_objId);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                        (uint8_t *) hsm_pake_context.spake_context, hsm_pake_context.spake_context_len,
                                        (uint8_t *) my_identity, my_identity_len, (uint8_t *) peer_identity, peer_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#else
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, SE05x_SPAKEDevice_A,
                                          hsm_pake_context.spake_objId, (uint8_t *) hsm_pake_context.spake_context,
                                          hsm_pake_context.spake_context_len, (uint8_t *) peer_identity, peer_identity_len,
                                          (uint8_t *) my_identity, my_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#endif

    error = se05x_set_key(w0in_id_p, w0in_mod, w0in_mod_len, kSSS_KeyPart_Default, kSSS_CipherType_AES);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = se05x_set_key(w1in_id_p, w1in_mod, w1in_mod_len, kSSS_KeyPart_Default, kSSS_CipherType_AES);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEInitCredentials(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                             hsm_pake_context.spake_objId, w0in_id_p, w1in_id_p, 0);
#else
    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, hsm_pake_context.spake_objId,
                                        w0in_id_p, w1in_id_p, 0);
#endif
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::PROVER;

exit:
    return error;
}
#endif

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::STARTED, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(*out_len >= point_size, error = CHIP_ERROR_INTERNAL);

#if !ENABLE_HSM_SPAKE_VERIFIER
    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        goto sw_rollback;
    }
#endif

#if ((CHIP_CRYPTO_HSM) && (!ENABLE_HSM_SPAKE_PROVER))
    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER)
    {
        goto sw_rollback;
    }
#endif

    error = Spake2p_ComputeRoundOne_HSM(&hsm_pake_context, role, pab, pab_len, out, out_len);
    if (CHIP_NO_ERROR == error)
    {
        state = CHIP_SPAKE2P_STATE::R1;
    }
    goto exit;

#if (!ENABLE_HSM_SPAKE_VERIFIER) || (!ENABLE_HSM_SPAKE_PROVER)
sw_rollback:
#endif

    error = Spake2p::ComputeRoundOne(pab, pab_len, out, out_len);

exit:
    return error;
}

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error   = CHIP_ERROR_INTERNAL;
    uint8_t pKeyKe[16] = {
        0,
    };
    size_t pkeyKeLen = sizeof(pKeyKe);

    VerifyOrExit(*out_len >= hash_size, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(in_len == point_size, error = CHIP_ERROR_INTERNAL);

#if !ENABLE_HSM_SPAKE_VERIFIER
    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        goto sw_rollback;
    }
#endif

#if !ENABLE_HSM_SPAKE_PROVER
    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER)
    {
        goto sw_rollback;
    }
#endif

    error = Spake2p_ComputeRoundTwo_HSM(&hsm_pake_context, role, in, in_len, out, out_len, pKeyKe, &pkeyKeLen);
    if (CHIP_NO_ERROR == error)
    {
        memcpy((Kae + 16), pKeyKe, pkeyKeLen);
        state = CHIP_SPAKE2P_STATE::R2;
    }

    goto exit;

#if ((!ENABLE_HSM_SPAKE_VERIFIER) || (!ENABLE_HSM_SPAKE_PROVER))
sw_rollback:
#endif

    error = Spake2p::ComputeRoundTwo(in, in_len, out, out_len);

exit:
    return error;
}

CHIP_ERROR Spake2pHSM_P256_SHA256_HKDF_HMAC::KeyConfirm(const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R2, error = CHIP_ERROR_INTERNAL);

#if !ENABLE_HSM_SPAKE_VERIFIER
    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        goto sw_rollback;
    }
#endif

#if !ENABLE_HSM_SPAKE_PROVER
    if (role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER)
    {
        goto sw_rollback;
    }
#endif

    error = Spake2p_KeyConfirm_HSM(&hsm_pake_context, role, in, in_len);
    if (CHIP_NO_ERROR == error)
    {
        state = CHIP_SPAKE2P_STATE::KC;
    }

    Spake2p_Finish_HSM(&hsm_pake_context);

    goto exit;

#if (!ENABLE_HSM_SPAKE_VERIFIER || !ENABLE_HSM_SPAKE_PROVER)
sw_rollback:
#endif

    error = Spake2p::KeyConfirm(in, in_len);

exit:
    return error;
}

} // namespace Crypto
} // namespace chip

#endif //#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))
