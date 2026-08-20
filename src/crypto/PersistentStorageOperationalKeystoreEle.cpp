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

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

#include "PersistentStorageOperationalKeystore.h"
#include "hsm_api.h"

constexpr int kKeyGroup              = 1;
constexpr int kPubkeySize            = 65;
constexpr unsigned int kKeyGroupSync = 0x1U << 7;

namespace chip {

using namespace chip::Crypto;

bool PersistentStorageOperationalKeystore::HasPendingOpKeypair() const
{
    return (mPendingFabricIndex != kUndefinedFabricIndex);
}

bool PersistentStorageOperationalKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    op_get_key_attr_args_t keyattr_args;
    hsm_err_t err;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);
    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        return true;
    }

    // get the key attributes, it shall fail if no such key
    memset(&keyattr_args, 0, sizeof(keyattr_args));
    keyattr_args.key_identifier = fabricIndex;
    err                         = hsm_get_key_attr(mEleManager->key_mgmt_hdl, &keyattr_args);
    if (err != HSM_NO_ERROR)
    {
        ChipLogDetail(Crypto, "No keypair for fabric: %d found. ELE returns: 0x%x\n", fabricIndex, err);
        return false;
    }
    else
    {
        ChipLogDetail(Crypto, "Found keypair for fabric: %d.\n", fabricIndex);
        return true;
    }
}

CHIP_ERROR PersistentStorageOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                       MutableByteSpan & outCertificateSigningRequest)
{
    op_generate_key_args_t key_gen_args;
    op_get_key_attr_args_t keyattr_args;
    uint32_t keyId;
    hsm_err_t hsm_err;
    CHIP_ERROR err;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Replace previous pending keypair
    ResetPendingKey();

    // Delete the key if already existed
    memset(&keyattr_args, 0, sizeof(keyattr_args));
    keyattr_args.key_identifier = fabricIndex;
    hsm_err                     = hsm_get_key_attr(mEleManager->key_mgmt_hdl, &keyattr_args);
    if (hsm_err == HSM_NO_ERROR)
    {
        ChipLogDetail(Crypto, "Keypair for fabric: %d exists, delete it...\n", fabricIndex);
        if (mEleManager->EleDeleteKey(fabricIndex) != HSM_NO_ERROR)
            return CHIP_ERROR_HSM;
    }

    // Generate new key
    memset(&key_gen_args, 0, sizeof(key_gen_args));
    keyId                       = fabricIndex;
    key_gen_args.key_identifier = &keyId;
    key_gen_args.key_group      = kKeyGroup;
    key_gen_args.key_lifetime   = HSM_SE_KEY_STORAGE_PERSISTENT;
    key_gen_args.key_usage =
        HSM_KEY_USAGE_SIGN_HASH | HSM_KEY_USAGE_VERIFY_HASH | HSM_KEY_USAGE_SIGN_MSG | HSM_KEY_USAGE_VERIFY_MSG;
    key_gen_args.permitted_algo = PERMITTED_ALGO_ECDSA_SHA256;
    // "sync" flag is not set so the key won't be committed to NVM immediately.
    // we will commit the key in the CommitOpKeypairForFabric().
    key_gen_args.flags         = 0;
    key_gen_args.key_type      = HSM_KEY_TYPE_ECC_NIST;
    key_gen_args.bit_key_sz    = HSM_KEY_SIZE_ECC_NIST_256;
    key_gen_args.key_lifecycle = (hsm_key_lifecycle_t) 0;
    hsm_err                    = hsm_generate_key(mEleManager->key_mgmt_hdl, &key_gen_args);
    ChipLogDetail(Crypto, "Generate new keypair returns: 0x%x\n", hsm_err);
    if (hsm_err != HSM_NO_ERROR)
        return CHIP_ERROR_HSM;

    // generate the CSR
    size_t csrLength = outCertificateSigningRequest.size();
    err              = mEleManager->EleGenerateCSR(keyId, outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return err;
    }

    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                            const Crypto::P256PublicKey & nocPublicKey)
{
    uint8_t pubkey[kPubkeySize];
    hsm_err_t hsmret = HSM_NO_ERROR;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    // public key size is 65
    pubkey[0] = 0x04;

    op_pub_key_recovery_args_t args = { 0 };
    args.key_identifier             = fabricIndex;
    args.out_key_size               = 64;
    args.out_key                    = &pubkey[1];
    hsmret                          = hsm_pub_key_recovery(mEleManager->key_store_hdl, &args);
    if (hsmret != HSM_NO_ERROR)
    {
        ChipLogDetail(Crypto, "recover public key failed. ret:0x%x\n", hsmret);
        return CHIP_ERROR_HSM;
    }

    if (memcmp(pubkey, nocPublicKey.ConstBytes(), sizeof(pubkey)))
    {
        ChipLogDetail(Crypto, "the public key being activated doesn't match pending key.\n");
        return CHIP_ERROR_INVALID_PUBLIC_KEY;
    }

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    hsm_err_t hsmret = HSM_NO_ERROR;
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Try to store persistent key. On failure, leave everything pending as-is
    op_manage_key_group_args_t keygroup_args;
    memset(&keygroup_args, 0, sizeof(keygroup_args));
    keygroup_args.key_group = kKeyGroup;
    keygroup_args.flags     = kKeyGroupSync;
    hsmret                  = hsm_manage_key_group(mEleManager->key_mgmt_hdl, &keygroup_args);
    if (hsmret != HSM_NO_ERROR)
    {
        ChipLogDetail(Crypto, "commit key failed. ret: 0x%x\n", hsmret);
        return CHIP_ERROR_HSM;
    }
    else
    {
        ChipLogDetail(Crypto, "commit key successfully.\n");
    }

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    mIsExternallyOwnedKeypair = false;
    mIsPendingKeypairActive   = false;
    mPendingFabricIndex       = kUndefinedFabricIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::ExportOpKeypairForFabric(FabricIndex fabricIndex,
                                                                          Crypto::P256SerializedKeypair & outKeypair)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PersistentStorageOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    hsm_err_t hsmret = HSM_NO_ERROR;
    op_get_key_attr_args_t keyattr_args;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Remove pending state if matching
    if (fabricIndex == mPendingFabricIndex)
    {
        ResetPendingKey();
        return CHIP_NO_ERROR;
    }

    // get the key attributes, it shall fail if no such key.
    memset(&keyattr_args, 0, sizeof(keyattr_args));
    keyattr_args.key_identifier = fabricIndex;
    hsmret                      = hsm_get_key_attr(mEleManager->key_mgmt_hdl, &keyattr_args);
    if (hsmret != HSM_NO_ERROR)
    {
        ChipLogDetail(Crypto, "No keypair for fabric: %d found. No need to delete\n", fabricIndex);
        return CHIP_NO_ERROR;
    }
    else
    {
        hsmret = mEleManager->EleDeleteKey(fabricIndex);
        if (hsmret != HSM_NO_ERROR)
            return CHIP_ERROR_HSM;
    }

    return CHIP_NO_ERROR;
}

void PersistentStorageOperationalKeystore::RevertPendingKeypair()
{
    // Just reset the pending key, it hasn't been stored into secure storage.
    ResetPendingKey();
}

CHIP_ERROR PersistentStorageOperationalKeystore::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                                   Crypto::P256ECDSASignature & outSignature) const
{
    uint8_t sig[kP256_ECDSA_Signature_Length_Raw];
    hsm_err_t hsmret = HSM_NO_ERROR;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    hsmret = mEleManager->EleSignMessage(fabricIndex, message.data(), message.size(), sig, kP256_ECDSA_Signature_Length_Raw);
    VerifyOrReturnError(hsmret == HSM_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(outSignature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    memcpy(outSignature.Bytes(), sig, kP256_ECDSA_Signature_Length_Raw);

    return CHIP_NO_ERROR;
}

Crypto::P256Keypair * PersistentStorageOperationalKeystore::AllocateEphemeralKeypairForCASE()
{
    return Platform::New<Crypto::P256Keypair>();
}

void PersistentStorageOperationalKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

CHIP_ERROR PersistentStorageOperationalKeystore::MigrateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                           OperationalKeystore & operationalKeystore) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace chip
