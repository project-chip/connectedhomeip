/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include "PSASpake2p.h"

#include "CHIPCryptoPALPSA.h"

#include <lib/support/CodeUtils.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::Init(const uint8_t * context, size_t context_len)
{
    Clear();

    VerifyOrReturnError(context_len <= sizeof(mContext), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(mContext, context, context_len);
    mContextLen = context_len;

    return CHIP_NO_ERROR;
}

void PSASpake2p_P256_SHA256_HKDF_HMAC::Clear()
{
    IgnoreUnusedVariable(psa_pake_abort(&mOperation));
    mOperation = psa_pake_operation_init();

    IgnoreUnusedVariable(psa_destroy_key(mKey));
    mKey = PSA_KEY_ID_NULL;
}

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len,
                                                           const uint8_t * peer_identity, size_t peer_identity_len,
                                                           const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                                           size_t Lin_len)
{
    VerifyOrReturnError(w0in_len <= kSpake2p_WS_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(Lin_len == kP256_Point_Length, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t password[kSpake2p_WS_Length + kP256_Point_Length];
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_pake_cipher_suite_t cp      = PSA_PAKE_CIPHER_SUITE_INIT;

    psa_pake_cs_set_algorithm(&cp, PSA_ALG_SPAKE2P_MATTER);
    psa_pake_cs_set_primitive(&cp, PSA_PAKE_PRIMITIVE(PSA_PAKE_PRIMITIVE_TYPE_ECC, PSA_ECC_FAMILY_SECP_R1, 256));

    memcpy(password + 0, w0in, w0in_len);
    memcpy(password + w0in_len, Lin, Lin_len);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&attributes, PSA_ALG_SPAKE2P_MATTER);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_SPAKE2P_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));

    psa_status_t status = psa_import_key(&attributes, password, w0in_len + Lin_len, &mKey);

    psa_reset_key_attributes(&attributes);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_setup(&mOperation, mKey, &cp);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    mRole  = PSA_PAKE_ROLE_SERVER;
    status = psa_pake_set_role(&mOperation, PSA_PAKE_ROLE_SERVER);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_set_peer(&mOperation, peer_identity, peer_identity_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_set_user(&mOperation, my_identity, my_identity_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_set_context(&mOperation, mContext, mContextLen);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::BeginProver(const uint8_t * my_identity, size_t my_identity_len,
                                                         const uint8_t * peer_identity, size_t peer_identity_len,
                                                         const uint8_t * w0sin, size_t w0sin_len, const uint8_t * w1sin,
                                                         size_t w1sin_len)
{
    VerifyOrReturnError(w0sin_len <= kSpake2p_WS_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(w1sin_len <= kSpake2p_WS_Length, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t password[kSpake2p_WS_Length * 2];
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_pake_cipher_suite_t cp      = PSA_PAKE_CIPHER_SUITE_INIT;

    psa_pake_cs_set_algorithm(&cp, PSA_ALG_SPAKE2P_MATTER);
    psa_pake_cs_set_primitive(&cp, PSA_PAKE_PRIMITIVE(PSA_PAKE_PRIMITIVE_TYPE_ECC, PSA_ECC_FAMILY_SECP_R1, 256));

    memcpy(password + 0, w0sin, w0sin_len);
    memcpy(password + w0sin_len, w1sin, w1sin_len);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&attributes, PSA_ALG_SPAKE2P_MATTER);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_SPAKE2P_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));

    psa_status_t status = psa_import_key(&attributes, password, w0sin_len + w1sin_len, &mKey);

    psa_reset_key_attributes(&attributes);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_setup(&mOperation, mKey, &cp);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    mRole  = PSA_PAKE_ROLE_CLIENT;
    status = psa_pake_set_role(&mOperation, PSA_PAKE_ROLE_CLIENT);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_set_user(&mOperation, my_identity, my_identity_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_set_peer(&mOperation, peer_identity, peer_identity_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_pake_set_context(&mOperation, mContext, mContextLen);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    VerifyOrReturnError(out_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    psa_status_t status;

    if (mRole == PSA_PAKE_ROLE_SERVER)
    {
        status = psa_pake_input(&mOperation, PSA_PAKE_STEP_KEY_SHARE, pab, pab_len);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    }

    status = psa_pake_output(&mOperation, PSA_PAKE_STEP_KEY_SHARE, out, *out_len, out_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len)
{
    VerifyOrReturnError(out_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    psa_status_t status;

    if (mRole == PSA_PAKE_ROLE_CLIENT)
    {
        status = psa_pake_input(&mOperation, PSA_PAKE_STEP_KEY_SHARE, in, in_len);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    }

    status = psa_pake_output(&mOperation, PSA_PAKE_STEP_CONFIRM, out, *out_len, out_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::KeyConfirm(const uint8_t * in, size_t in_len)
{
    psa_status_t status = psa_pake_input(&mOperation, PSA_PAKE_STEP_CONFIRM, in, in_len);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSASpake2p_P256_SHA256_HKDF_HMAC::GetKeys(SessionKeystore & keystore, HkdfKeyHandle & key)
{
    auto & keyId = key.AsMutable<psa_key_id_t>();

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_type(&attributes, PSA_KEY_TYPE_DERIVE);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&attributes, PSA_ALG_HKDF(PSA_ALG_SHA_256));

    psa_status_t status = psa_pake_get_shared_key(&(mOperation), &attributes, &keyId);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2pVerifier::Generate(uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t setupPin)
{
    psa_status_t status                      = PSA_SUCCESS;
    psa_key_attributes_t attributes          = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t passwordKey                 = PSA_KEY_ID_NULL;
    psa_key_derivation_operation_t operation = PSA_KEY_DERIVATION_OPERATION_INIT;
    psa_key_id_t spakeKey                    = PSA_KEY_ID_NULL;
    uint8_t verifier[kP256_FE_Length + kP256_Point_Length];
    size_t verifierLen;

    // Prepare password key
    uint8_t password[sizeof(uint32_t)];
    Encoding::LittleEndian::Put32(password, setupPin);

    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&attributes, PSA_ALG_PBKDF2_HMAC(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_PASSWORD);

    status = psa_import_key(&attributes, password, sizeof(password), &passwordKey);
    psa_reset_key_attributes(&attributes);
    VerifyOrExit(status == PSA_SUCCESS, );

    // Run PBKDF
    status = psa_key_derivation_setup(&operation, PSA_ALG_PBKDF2_HMAC(PSA_ALG_SHA_256));
    VerifyOrExit(status == PSA_SUCCESS, );

    status = psa_key_derivation_input_integer(&operation, PSA_KEY_DERIVATION_INPUT_COST, pbkdf2IterCount);
    VerifyOrExit(status == PSA_SUCCESS, );

    status = psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_SALT, salt.data(), salt.size());
    VerifyOrExit(status == PSA_SUCCESS, );

    status = psa_key_derivation_input_key(&operation, PSA_KEY_DERIVATION_INPUT_PASSWORD, passwordKey);
    VerifyOrExit(status == PSA_SUCCESS, );

    attributes = psa_key_attributes_init();
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_EXPORT);
    psa_set_key_algorithm(&attributes, PSA_ALG_SPAKE2P_MATTER);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_SPAKE2P_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_FE_Length * 8);

    status = psa_key_derivation_output_key(&attributes, &operation, &spakeKey);
    psa_reset_key_attributes(&attributes);
    VerifyOrExit(status == PSA_SUCCESS, );

    // Export verifier as raw bytes
    status = psa_export_public_key(spakeKey, verifier, sizeof(verifier), &verifierLen);

exit:
    psa_key_derivation_abort(&operation);
    psa_destroy_key(passwordKey);
    psa_destroy_key(spakeKey);

    if (status != PSA_SUCCESS)
    {
        ChipLogError(Crypto, "PSA error: %d", static_cast<int>(status));
        return CHIP_ERROR_INTERNAL;
    }

    return Deserialize(ByteSpan(verifier, verifierLen));
}

} // namespace Crypto
} // namespace chip
