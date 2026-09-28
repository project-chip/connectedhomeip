/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/**
 * Minimal PSA Crypto surface for SiWx917 tinycrypt builds.
 *
 * mbedTLS 4 PK/X.509 call PSA for hash, key import, and ECDSA verify, but the
 * tinycrypt flavor does not compile the TF-PSA-Crypto core.
 */

#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS

#include <psa/crypto.h>

#include <mbedtls/private/sha256.h>
#include <tinycrypt/ecc_dsa.h>

#include <stdint.h>
#include <string.h>

enum
{
    kMaxHashOps  = 4,
    kMaxKeys     = 8,
    kP256PubLen = 65,
    kP256SigLen = 64,
    kP256Bits    = 256,
};

typedef struct
{
    uint8_t in_use;
    psa_algorithm_t alg;
    mbedtls_sha256_context sha256;
} HashSlot;

typedef struct
{
    uint8_t in_use;
    psa_key_type_t type;
    size_t bits;
    size_t len;
    uint8_t data[kP256PubLen];
} KeySlot;

static HashSlot sHashOps[kMaxHashOps];
static KeySlot sKeys[kMaxKeys];

static uint32_t HashOpId(const psa_hash_operation_t * operation)
{
    uint32_t id = 0;
    memcpy(&id, operation, sizeof(id));
    return id;
}

static void SetHashOpId(psa_hash_operation_t * operation, uint32_t id)
{
    memcpy(operation, &id, sizeof(id));
}

static HashSlot * HashSlotFromId(uint32_t id)
{
    if ((id == 0) || (id > kMaxHashOps))
    {
        return NULL;
    }

    HashSlot * slot = &sHashOps[id - 1];
    if (!slot->in_use)
    {
        return NULL;
    }

    return slot;
}

static KeySlot * KeySlotFromId(mbedtls_svc_key_id_t key)
{
    if (mbedtls_svc_key_id_is_null(key) || (key > kMaxKeys))
    {
        return NULL;
    }

    KeySlot * slot = &sKeys[key - 1];
    if (!slot->in_use)
    {
        return NULL;
    }

    return slot;
}

static psa_status_t MbedtlsToPsa(int ret)
{
    if (ret == 0)
    {
        return PSA_SUCCESS;
    }

    return PSA_ERROR_GENERIC_ERROR;
}

psa_status_t psa_crypto_init(void)
{
    return PSA_SUCCESS;
}

psa_status_t psa_hash_abort(psa_hash_operation_t * operation)
{
    if (operation == NULL)
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    const uint32_t id = HashOpId(operation);
    if (id == 0)
    {
        return PSA_SUCCESS;
    }

    HashSlot * slot = HashSlotFromId(id);
    if (slot != NULL)
    {
        mbedtls_sha256_free(&slot->sha256);
        memset(slot, 0, sizeof(*slot));
    }

    SetHashOpId(operation, 0);
    return PSA_SUCCESS;
}

psa_status_t psa_hash_setup(psa_hash_operation_t * operation, psa_algorithm_t alg)
{
    if ((operation == NULL) || !PSA_ALG_IS_HASH(alg))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (HashOpId(operation) != 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    if ((alg != PSA_ALG_SHA_256) && (alg != PSA_ALG_SHA_224))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    size_t i = 0;
    for (i = 0; i < kMaxHashOps; i++)
    {
        if (!sHashOps[i].in_use)
        {
            break;
        }
    }

    if (i == kMaxHashOps)
    {
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    HashSlot * slot = &sHashOps[i];
    memset(slot, 0, sizeof(*slot));
    mbedtls_sha256_init(&slot->sha256);

    const int is224 = (alg == PSA_ALG_SHA_224) ? 1 : 0;
    const int ret   = mbedtls_sha256_starts(&slot->sha256, is224);
    if (ret != 0)
    {
        mbedtls_sha256_free(&slot->sha256);
        return MbedtlsToPsa(ret);
    }

    slot->alg    = alg;
    slot->in_use = 1;
    SetHashOpId(operation, (uint32_t) (i + 1));
    return PSA_SUCCESS;
}

psa_status_t psa_hash_update(psa_hash_operation_t * operation, const uint8_t * input, size_t input_length)
{
    HashSlot * slot = NULL;

    if ((operation == NULL) || ((input == NULL) && (input_length != 0)))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    slot = HashSlotFromId(HashOpId(operation));
    if (slot == NULL)
    {
        return PSA_ERROR_BAD_STATE;
    }

    return MbedtlsToPsa(mbedtls_sha256_update(&slot->sha256, input, input_length));
}

psa_status_t psa_hash_finish(psa_hash_operation_t * operation, uint8_t * hash, size_t hash_size, size_t * hash_length)
{
    HashSlot * slot     = NULL;
    size_t expected_len = 0;
    unsigned char tmp[32];
    psa_status_t status = PSA_ERROR_GENERIC_ERROR;

    if ((operation == NULL) || (hash == NULL) || (hash_length == NULL))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    slot = HashSlotFromId(HashOpId(operation));
    if (slot == NULL)
    {
        return PSA_ERROR_BAD_STATE;
    }

    expected_len = PSA_HASH_LENGTH(slot->alg);
    if (hash_size < expected_len)
    {
        (void) psa_hash_abort(operation);
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    status = MbedtlsToPsa(mbedtls_sha256_finish(&slot->sha256, tmp));
    if (status == PSA_SUCCESS)
    {
        memcpy(hash, tmp, expected_len);
        *hash_length = expected_len;
    }

    (void) psa_hash_abort(operation);
    return status;
}

psa_status_t psa_hash_compute(psa_algorithm_t alg, const uint8_t * input, size_t input_length, uint8_t * hash, size_t hash_size,
                              size_t * hash_length)
{
    psa_hash_operation_t operation = PSA_HASH_OPERATION_INIT;
    psa_status_t status            = psa_hash_setup(&operation, alg);

    if (status != PSA_SUCCESS)
    {
        return status;
    }

    status = psa_hash_update(&operation, input, input_length);
    if (status != PSA_SUCCESS)
    {
        (void) psa_hash_abort(&operation);
        return status;
    }

    return psa_hash_finish(&operation, hash, hash_size, hash_length);
}

psa_status_t psa_hash_clone(const psa_hash_operation_t * source_operation, psa_hash_operation_t * target_operation)
{
    HashSlot * src = NULL;
    size_t i       = 0;

    if ((source_operation == NULL) || (target_operation == NULL))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (HashOpId(target_operation) != 0)
    {
        return PSA_ERROR_BAD_STATE;
    }

    src = HashSlotFromId(HashOpId(source_operation));
    if (src == NULL)
    {
        return PSA_ERROR_BAD_STATE;
    }

    for (i = 0; i < kMaxHashOps; i++)
    {
        if (!sHashOps[i].in_use)
        {
            break;
        }
    }

    if (i == kMaxHashOps)
    {
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    sHashOps[i]        = *src;
    sHashOps[i].in_use = 1;
    mbedtls_sha256_init(&sHashOps[i].sha256);
    mbedtls_sha256_clone(&sHashOps[i].sha256, &src->sha256);
    SetHashOpId(target_operation, (uint32_t) (i + 1));
    return PSA_SUCCESS;
}

psa_status_t psa_destroy_key(mbedtls_svc_key_id_t key)
{
    KeySlot * slot = NULL;

    if (mbedtls_svc_key_id_is_null(key))
    {
        return PSA_SUCCESS;
    }

    slot = KeySlotFromId(key);
    if (slot == NULL)
    {
        return PSA_ERROR_INVALID_HANDLE;
    }

    memset(slot, 0, sizeof(*slot));
    return PSA_SUCCESS;
}

psa_status_t psa_import_key(const psa_key_attributes_t * attributes, const uint8_t * data, size_t data_length,
                            mbedtls_svc_key_id_t * key)
{
    psa_key_type_t type = 0;
    size_t bits         = 0;
    size_t i            = 0;

    if ((attributes == NULL) || (data == NULL) || (key == NULL) || (data_length == 0))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    type = psa_get_key_type(attributes);
    bits = psa_get_key_bits(attributes);

    *key = MBEDTLS_SVC_KEY_ID_INIT;

    if (!PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(type) || (PSA_KEY_TYPE_ECC_GET_FAMILY(type) != PSA_ECC_FAMILY_SECP_R1))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (bits == 0)
    {
        bits = kP256Bits;
    }

    if ((bits != kP256Bits) || (data_length != kP256PubLen) || (data[0] != 0x04))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    for (i = 0; i < kMaxKeys; i++)
    {
        if (!sKeys[i].in_use)
        {
            break;
        }
    }

    if (i == kMaxKeys)
    {
        return PSA_ERROR_INSUFFICIENT_MEMORY;
    }

    sKeys[i].in_use = 1;
    sKeys[i].type   = type;
    sKeys[i].bits   = bits;
    sKeys[i].len    = data_length;
    memcpy(sKeys[i].data, data, data_length);
    *key = (mbedtls_svc_key_id_t) (i + 1);
    return PSA_SUCCESS;
}

psa_status_t psa_get_key_attributes(mbedtls_svc_key_id_t key, psa_key_attributes_t * attributes)
{
    KeySlot * slot = KeySlotFromId(key);

    if (attributes == NULL)
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    psa_reset_key_attributes(attributes);
    if (slot == NULL)
    {
        return PSA_ERROR_INVALID_HANDLE;
    }

    psa_set_key_type(attributes, slot->type);
    psa_set_key_bits(attributes, slot->bits);
    return PSA_SUCCESS;
}

psa_status_t psa_export_public_key(mbedtls_svc_key_id_t key, uint8_t * data, size_t data_size, size_t * data_length)
{
    KeySlot * slot = KeySlotFromId(key);

    if ((data == NULL) || (data_length == NULL))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (slot == NULL)
    {
        return PSA_ERROR_INVALID_HANDLE;
    }

    if (data_size < slot->len)
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy(data, slot->data, slot->len);
    *data_length = slot->len;
    return PSA_SUCCESS;
}

psa_status_t psa_verify_hash(mbedtls_svc_key_id_t key, psa_algorithm_t alg, const uint8_t * hash, size_t hash_length,
                             const uint8_t * signature, size_t signature_length)
{
    KeySlot * slot = KeySlotFromId(key);

    if ((hash == NULL) || (signature == NULL))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (slot == NULL)
    {
        return PSA_ERROR_INVALID_HANDLE;
    }

    if (!PSA_ALG_IS_ECDSA(alg))
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if ((slot->len != kP256PubLen) || (signature_length != kP256SigLen) || (hash_length == 0))
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (uECC_verify(slot->data + 1, hash, (unsigned int) hash_length, signature) != UECC_SUCCESS)
    {
        return PSA_ERROR_INVALID_SIGNATURE;
    }

    return PSA_SUCCESS;
}
