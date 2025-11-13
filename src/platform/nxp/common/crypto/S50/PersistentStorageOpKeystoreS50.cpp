/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/SafePointerCast.h>

#include "PersistentStorageOpKeystoreS50.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
#include "ELSFactoryData.h"
#if defined(__cplusplus)
}
#endif /* __cplusplus */

namespace chip {

using namespace chip::Crypto;

static constexpr size_t kPrivateKeyBlobLength = Crypto::kP256_PrivateKey_Length + ELS_BLOB_METADATA_SIZE + ELS_WRAP_OVERHEAD;

// In mbedTLS 3.0.0 direct access to structure fields was replaced with using MBEDTLS_PRIVATE macro.
#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
#define CHIP_CRYPTO_PAL_PRIVATE(x) MBEDTLS_PRIVATE(x)
#else
#define CHIP_CRYPTO_PAL_PRIVATE(x) x
#endif

static inline const mbedtls_ecp_keypair * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const mbedtls_ecp_keypair *>(context);
}

CHIP_ERROR P256KeypairNXP::ExportBlob(P256SerializedKeypairNXP & output) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    status_t status                          = STATUS_SUCCESS;
    size_t blobSize                          = kPrivateKeyBlobLength;
    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_KGSRC }
    };
    mcuxClEls_KeyIndex_t key_index = MCUXCLELS_KEY_SLOTS;

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    /* Extract plain ECC private key */
    uint8_t privkey[kP256_PrivateKey_Length];
    mbedtls_mpi_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(d), Uint8::to_uchar(privkey), sizeof(privkey));
    PLOG_DEBUG_BUFFER("plain private key", privkey, kP256_PrivateKey_Length);
    els_enable();

    /* Import plain DAC key into S50 */
    status = import_plain_key_into_els(privkey, kP256_PrivateKey_Length, plain_key_properties, &key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("import_plain_key_into_els failed: 0x%08x", status);

    /* ELS generate key blob. The blob created here is one that can be directly imported into ELS again. */
    status = export_key_from_els(key_index, output.Bytes(), &blobSize);
    STATUS_SUCCESS_OR_EXIT_MSG("export_key_from_els failed: 0x%08x", status);

    status = els_delete_key(key_index);

    return CHIP_NO_ERROR;
exit:
    status = els_delete_key(key_index);
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR P256KeypairNXP::ImportBlob(P256SerializedKeypairNXP & input)
{
    /* Import blob in S50 is done in SignWithOpKeypair as we need to know the keyslot to sign the message */
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PersistentStorageOpKeystoreS50::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                             Crypto::P256ECDSASignature & outSignature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    status_t status  = STATUS_SUCCESS;

    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        // We have an override key: sign with it!
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    uint8_t els_key_blob[kPrivateKeyBlobLength];
    uint16_t els_key_blob_size = sizeof(els_key_blob);
    uint8_t digest[kSHA256_Hash_Length];

    mcuxClEls_KeyIndex_t key_index           = MCUXCLELS_KEY_SLOTS;
    mcuxClEls_KeyProp_t plain_key_properties = {
        .word = { .value = MCUXCLELS_KEYPROPERTY_VALUE_SECURE | MCUXCLELS_KEYPROPERTY_VALUE_PRIVILEGED |
                      MCUXCLELS_KEYPROPERTY_VALUE_KEY_SIZE_256 | MCUXCLELS_KEYPROPERTY_VALUE_KGSRC }
    };

    mcuxClEls_EccSignOption_t sign_options = { 0 };
    mcuxClEls_EccByte_t ecc_signature[MCUXCLELS_ECC_SIGNATURE_SIZE];
    uint8_t public_key[64] = { 0 };
    size_t public_key_size = sizeof(public_key);

    error =
        mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), els_key_blob, els_key_blob_size);

    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        error = CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    ReturnErrorOnFailure(error);

    PLOG_DEBUG_BUFFER("els_key_blob", els_key_blob, els_key_blob_size);

    /* Calculate message HASH to sign */
    memset(&digest[0], 0, sizeof(digest));
    error = Hash_SHA256(message.data(), message.size(), &digest[0]);
    if (error != CHIP_NO_ERROR)
    {
        return error;
    }
    PLOG_DEBUG_BUFFER("HASH", digest, kSHA256_Hash_Length);

    /* Import blob DAC key into SE50 (reserved key slot) */
    status = import_die_int_wrapped_key_into_els(els_key_blob, els_key_blob_size, plain_key_properties, &key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("import_die_int_wrapped_key_into_els failed: 0x%08x", status);

    /* For ECC keys that were created from plain key material, there is the
     neceessity to convert them to a key. Converting to a key also yields the public key.
     The conversion can be done either before re-wrapping (when importing the plain key)
     or after (when importing the blob).*/
    status = els_keygen(key_index, &public_key[0], &public_key_size);
    STATUS_SUCCESS_OR_EXIT_MSG("els_keygen failed: 0x%08x", status);

    /* The key is usable for signing. */
    PLOG_DEBUG_BUFFER("public_key", public_key, public_key_size);

    /* ECC sign message hash with the key index slot reserved during the blob importation */
    status = ELS_sign_hash(digest, ecc_signature, &sign_options, key_index);
    STATUS_SUCCESS_OR_EXIT_MSG("ELS_sign_hash failed: 0x%08x", status);

    /* Delete SE50 key with the index slot reserved during the blob importation (free key slot) */
    els_delete_key(key_index);

    /* Generate MutableByteSpan with ECC signature and ECC signature size */
    outSignature.SetLength(MCUXCLELS_ECC_SIGNATURE_SIZE);
    memcpy(outSignature.Bytes(), ecc_signature, outSignature.Length());
    PLOG_DEBUG_BUFFER("ECDSA signature", ecc_signature, MCUXCLELS_ECC_SIGNATURE_SIZE);
    return CHIP_NO_ERROR;
exit:
    els_delete_key(key_index);
    return CHIP_ERROR_INVALID_SIGNATURE;
}

} // namespace chip
