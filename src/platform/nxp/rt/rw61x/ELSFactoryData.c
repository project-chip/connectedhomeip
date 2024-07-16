/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ELSFactoryData.h"

uint8_t * append_u32(uint8_t * pos, uint32_t val)
{
    *pos++ = 4;
    *pos++ = (val >> 24) & 0xFF;
    *pos++ = (val >> 16) & 0xFF;
    *pos++ = (val >> 8) & 0xFF;
    *pos++ = val & 0xFF;
    return pos;
}

uint8_t * append_u16(uint8_t * pos, uint32_t val)
{
    *pos++ = 2;
    *pos++ = (val >> 8) & 0xFF;
    *pos++ = val & 0xFF;
    return pos;
}

void write_uint32_msb_first(uint8_t * pos, uint32_t data)
{
    pos[0] = ((data) >> 24) & 0xFF;
    pos[1] = ((data) >> 16) & 0xFF;
    pos[2] = ((data) >> 8) & 0xFF;
    pos[3] = ((data) >> 0) & 0xFF;
}

void printf_buffer(const char * name, const unsigned char * buffer, size_t size)
{
#define PP_BYTES_PER_LINE (32)
    char line_buffer[PP_BYTES_PER_LINE * 2 + 2];
    const unsigned char * pos = buffer;
    size_t remaining          = size;
    while (remaining > 0)
    {
        size_t block_size = remaining > PP_BYTES_PER_LINE ? PP_BYTES_PER_LINE : remaining;
        uint32_t len      = 0;
        for (size_t i = 0; i < block_size; i++)
        {
            line_buffer[len++] = nibble_to_char[((*pos) & 0xf0) >> 4];
            line_buffer[len++] = nibble_to_char[(*pos++) & 0x0f];
        }
        line_buffer[len++] = '\n';
        line_buffer[len++] = '\0';
        PRINTF("%s (%p): %s", name, pos, line_buffer);
        remaining -= block_size;
    }
}

uint32_t get_required_keyslots(mcuxClEls_KeyProp_t prop)
{
    return prop.bits.ksize == MCUXCLELS_KEYPROPERTY_KEY_SIZE_128 ? 1U : 2U;
}

bool els_is_active_keyslot(mcuxClEls_KeyIndex_t keyIdx)
{
    mcuxClEls_KeyProp_t key_properties;
    key_properties.word.value = ((const volatile uint32_t *) (&ELS->ELS_KS0))[keyIdx];
    return key_properties.bits.kactv;
}

status_t els_enable()
{
    PLOG_INFO("Enabling ELS...");
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_Enable_Async());

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Enable_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Enable_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_get_key_properties(mcuxClEls_KeyIndex_t key_index, mcuxClEls_KeyProp_t * key_properties)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_GetKeyProperties(key_index, key_properties));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_GetKeyProperties) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_GetKeyProperties failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

mcuxClEls_KeyIndex_t els_get_free_keyslot(uint32_t required_keyslots)
{
    for (mcuxClEls_KeyIndex_t keyIdx = 0U; keyIdx <= (MCUXCLELS_KEY_SLOTS - required_keyslots); keyIdx++)
    {
        bool is_valid_keyslot = true;
        for (uint32_t i = 0U; i < required_keyslots; i++)
        {
            if (els_is_active_keyslot(keyIdx + i))
            {
                is_valid_keyslot = false;
                break;
            }
        }

        if (is_valid_keyslot)
        {
            return keyIdx;
        }
    }
    return MCUXCLELS_KEY_SLOTS;
}

status_t els_derive_key(mcuxClEls_KeyIndex_t src_key_index, mcuxClEls_KeyProp_t key_prop, const uint8_t * dd,
                        mcuxClEls_KeyIndex_t * dst_key_index)
{
    uint32_t required_keyslots = get_required_keyslots(key_prop);

    *dst_key_index = els_get_free_keyslot(required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_Ckdf_Sp800108_Async(src_key_index, *dst_key_index, key_prop, dd));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Ckdf_Sp800108_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Ckdf_Sp800108_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_delete_key(mcuxClEls_KeyIndex_t key_index)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_KeyDelete_Async(key_index));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_KeyDelete_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_KeyDelete_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_import_key(const uint8_t * wrapped_key, size_t wrapped_key_size, mcuxClEls_KeyProp_t key_prop,
                        mcuxClEls_KeyIndex_t unwrap_key_index, mcuxClEls_KeyIndex_t * dst_key_index)
{
    uint32_t required_keyslots = get_required_keyslots(key_prop);
    *dst_key_index             = els_get_free_keyslot(required_keyslots);

    if (!(*dst_key_index < MCUXCLELS_KEY_SLOTS))
    {
        PLOG_ERROR("no free keyslot found");
        return STATUS_ERROR_GENERIC;
    }

    mcuxClEls_KeyImportOption_t options;
    options.bits.kfmt = MCUXCLELS_KEYIMPORT_KFMT_RFC3394;
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token, mcuxClEls_KeyImport_Async(options, wrapped_key, wrapped_key_size, unwrap_key_index, *dst_key_index));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_KeyImport_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_KeyImport_Async failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08lx", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}

status_t els_keygen(mcuxClEls_KeyIndex_t key_index, uint8_t * public_key, size_t * public_key_size)
{
    status_t status = STATUS_SUCCESS;

    mcuxClEls_EccKeyGenOption_t key_gen_options;
    key_gen_options.word.value    = 0u;
    key_gen_options.bits.kgsign   = MCUXCLELS_ECC_PUBLICKEY_SIGN_DISABLE;
    key_gen_options.bits.kgsrc    = MCUXCLELS_ECC_OUTPUTKEY_DETERMINISTIC;
    key_gen_options.bits.skip_pbk = MCUXCLELS_ECC_GEN_PUBLIC_KEY;

    mcuxClEls_KeyProp_t key_properties;
    status = els_get_key_properties(key_index, &key_properties);
    STATUS_SUCCESS_OR_EXIT_MSG("get_key_properties failed: 0x%08x", status);

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token,
        mcuxClEls_EccKeyGen_Async(key_gen_options, (mcuxClEls_KeyIndex_t) 0, key_index, key_properties, NULL, &public_key[0]));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccKeyGen_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PRINTF("Css_EccKeyGen_Async failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PRINTF("Css_EccKeyGen_Async WaitForOperation failed: 0x%08lx\r\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
exit:
    return status;
}

status_t calculate_psa_import_blob_cmac(uint8_t * psa_import_blob, size_t psa_import_blob_length_before_mac,
                                        size_t psa_import_blob_size)
{
    status_t status                    = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t mac_key_index = MCUXCLELS_KEY_SLOTS;

    assert(psa_import_blob_size >= psa_import_blob_length_before_mac + AES_BLOCK_SIZE);

    uint8_t * pos                        = &psa_import_blob[psa_import_blob_length_before_mac];
    uint8_t mac[AES_BLOCK_SIZE]          = { 0 };
    uint32_t missing_bytes_to_fill_block = AES_BLOCK_SIZE - (psa_import_blob_length_before_mac % AES_BLOCK_SIZE);

    mcuxClEls_CmacOption_t cmac_options = { 0U };
    cmac_options.bits.initialize        = MCUXCLELS_CMAC_INITIALIZE_ENABLE;
    cmac_options.bits.finalize          = MCUXCLELS_CMAC_FINALIZE_ENABLE;
    cmac_options.bits.extkey            = MCUXCLELS_CMAC_EXTERNAL_KEY_DISABLE;
    // ELS needs us to pad the message, it does not do that itself :-(
    if (missing_bytes_to_fill_block != 0)
    {
        memset(pos, 0, missing_bytes_to_fill_block);
        *pos = 0x80;
    }

    PLOG_INFO("Deriving cmac key for integrity protection on key blob...");
    status = els_derive_key(DIE_INT_MK_SK_INDEX, mac_key_prop, ckdf_derivation_data_mac, &mac_key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(
        result, token,
        mcuxClEls_Cmac_Async(cmac_options, mac_key_index, NULL, 0, psa_import_blob, psa_import_blob_length_before_mac, mac));

    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_Cmac_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_Cmac_Async failed: 0x%08x", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed: 0x%08x\n", result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    status = els_delete_key(mac_key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);

    memcpy(pos, mac, sizeof(mac));
exit:
    return status;
}

status_t create_psa_import_blob(const uint8_t * els_key_blob, size_t els_key_blob_size, const psa_key_attributes_t * attributes,
                                uint8_t * output, size_t * output_size)
{
    assert(els_key_blob_size <= 48);
    assert(sizeof(key_blob_magic) < 0x80);

    status_t status = STATUS_SUCCESS;
    // clang-format off
    size_t required_output_size = 0
        + 2 + sizeof(key_blob_magic)
        + 2 + 4                    // key_id
        + 2 + 4                    // algorithms
        + 2 + 4                    // usage
        + 2 + 2                    // type
        + 2 + 4                    // bits
        + 2 + 4                    // lifetime
        + 2 + 4                    // device lifecycle
        + 2 + 4                    // wrapping key id
        + 2 + 4                    // wrapping algorithm
        + 2 + 4                    // signing key id
        + 2 + 4                    // signing algorithm
        + 2 + els_key_blob_size    // key blob from S50
        + 2 + AES_BLOCK_SIZE;      // CMAC
    // clang-format on

    if (*output_size < required_output_size)
    {
        PLOG_ERROR("key blob buffer too small");
        return STATUS_ERROR_GENERIC;
    }
    *output_size = required_output_size;

    uint32_t key_id           = psa_get_key_id(attributes);
    uint32_t key_alg          = psa_get_key_algorithm(attributes);
    uint32_t key_usage        = psa_get_key_usage_flags(attributes);
    uint16_t key_type         = psa_get_key_type(attributes);
    uint32_t key_bits         = psa_get_key_bits(attributes);
    uint32_t key_lifetime     = psa_get_key_lifetime(attributes);
    uint32_t device_lifecycle = 0x1; // 0x01: OPEN, 0x02: CLOSED, 0x04: CLOSED_LOCKED
    uint32_t wrapping_key_id  = NXP_DIE_INT_IMPORT_KEK_SK;
    uint32_t signing_key_id   = NXP_DIE_INT_IMPORT_AUTH_SK;

    PLOG_INFO("Creating key blob...");
    uint8_t * pos = output;

    *pos++ = 0x40;
    *pos++ = sizeof(key_blob_magic);
    memcpy(pos, key_blob_magic, sizeof(key_blob_magic));
    pos += sizeof(key_blob_magic);

    *pos++ = 0x41;
    pos    = append_u32(pos, key_id);

    *pos++ = 0x42;
    pos    = append_u32(pos, key_alg);

    *pos++ = 0x43;
    pos    = append_u32(pos, key_usage);

    *pos++ = 0x44;
    pos    = append_u16(pos, key_type);

    *pos++ = 0x45;
    pos    = append_u32(pos, key_bits);

    *pos++ = 0x46;
    pos    = append_u32(pos, key_lifetime);

    *pos++ = 0x47;
    pos    = append_u32(pos, device_lifecycle);

    *pos++ = 0x50;
    pos    = append_u32(pos, wrapping_key_id);

    *pos++ = 0x51;
    pos    = append_u32(pos, 0x01); // ELS RFC3394 wrapping

    *pos++ = 0x53;
    pos    = append_u32(pos, signing_key_id);

    *pos++ = 0x54;
    pos    = append_u32(pos, 0x01); // CMAC

    *pos++ = 0x55;
    *pos++ = els_key_blob_size;
    memcpy(pos, els_key_blob, els_key_blob_size);
    pos += els_key_blob_size;

    // signature
    *pos++                                   = 0x5E;
    *pos++                                   = AES_BLOCK_SIZE;
    size_t psa_import_blob_length_before_mac = pos - output;

    status = calculate_psa_import_blob_cmac(output, psa_import_blob_length_before_mac, *output_size);
    return status;
}

status_t import_die_int_wrapped_key_into_els(const uint8_t * wrapped_key, size_t wrapped_key_size,
                                             mcuxClEls_KeyProp_t key_properties, mcuxClEls_KeyIndex_t * index_output)
{
    status_t status                   = STATUS_SUCCESS;
    mcuxClEls_KeyIndex_t index_unwrap = MCUXCLELS_KEY_SLOTS;

    PLOG_INFO("Deriving wrapping key for import of die_int wrapped key on ELS...");
    status = els_derive_key(DIE_INT_MK_SK_INDEX, wrap_out_key_prop, ckdf_derivation_data_wrap_out, &index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("derive_key failed: 0x%08x", status);

    status = els_import_key(wrapped_key, wrapped_key_size, key_properties, index_unwrap, index_output);
    STATUS_SUCCESS_OR_EXIT_MSG("import_wrapped_key failed: 0x%08x", status);

    status = els_delete_key(index_unwrap);
    STATUS_SUCCESS_OR_EXIT_MSG("delete_key failed: 0x%08x", status);
    index_unwrap = MCUXCLELS_KEY_SLOTS;

exit:
    if (index_unwrap < MCUXCLELS_KEY_SLOTS)
    {
        (void) els_delete_key(index_unwrap);
    }
    return status;
}

status_t ELS_sign_hash(uint8_t * digest, mcuxClEls_EccByte_t * ecc_signature, mcuxClEls_EccSignOption_t * sign_options,
                       mcuxClEls_KeyIndex_t key_index)
{
    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token,
                                     mcuxClEls_EccSign_Async(       // Perform signature generation.
                                         *sign_options,             // Set the prepared configuration.
                                         key_index,                 // Set index of private key in keystore.
                                         digest, NULL, (size_t) 0U, // Pre-hashed data to sign. Note that inputLength parameter is
                                                                    // ignored since pre-hashed data has a fixed length.
                                         ecc_signature // Output buffer, which the operation will write the signature to.
                                         ));
    PLOG_DEBUG_BUFFER("mcuxClEls_EccSign_Async ecc_signature", ecc_signature, MCUXCLELS_ECC_SIGNATURE_SIZE);
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_EccSign_Async) != token) || (MCUXCLELS_STATUS_OK_WAIT != result))
    {
        PLOG_ERROR("mcuxClEls_EccSign_Async failed. token: 0x%08x, result: 0x%08x", token, result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();

    MCUX_CSSL_FP_FUNCTION_CALL_BEGIN(result, token, mcuxClEls_WaitForOperation(MCUXCLELS_ERROR_FLAGS_CLEAR));
    if ((MCUX_CSSL_FP_FUNCTION_CALLED(mcuxClEls_WaitForOperation) != token) || (MCUXCLELS_STATUS_OK != result))
    {
        PLOG_ERROR("mcuxClEls_WaitForOperation failed. token: 0x%08x, result: 0x%08x", token, result);
        return STATUS_ERROR_GENERIC;
    }
    MCUX_CSSL_FP_FUNCTION_CALL_END();
    return STATUS_SUCCESS;
}
