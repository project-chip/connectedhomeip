/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PersistentStorageOpKeystoreS200.h"

#include "sss_crypto.h"

namespace chip {

using namespace chip::Crypto;

static inline sss_sscp_object_t * to_keypair(P256KeypairContext * context)
{
    return SafePointerCast<sss_sscp_object_t *>(context);
}

CHIP_ERROR P256KeypairSSS::ExportBlob(P256SerializedKeypairSSS & output) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    size_t keyBlobLen = output.Capacity();
    auto res          = sss_sscp_key_store_export_key(&g_keyStore, keypair, output.Bytes(), &keyBlobLen, kSSS_blobType_ELKE_blob);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    output.SetLength(keyBlobLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256KeypairSSS::ImportBlob(P256SerializedKeypairSSS & input)
{
    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    if (false == mInitialized)
    {
        auto res = sss_sscp_key_object_init(keypair, &g_keyStore);
        VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

        /* Allocate key handle */
        res = sss_sscp_key_object_allocate_handle(keypair, 0x0u, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P,
                                                  3 * kP256_PrivateKey_Length, SSS_KEYPROP_OPERATION_ASYM);
        VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit((sss_sscp_key_store_import_key(&g_keyStore, keypair, input.Bytes(), input.Length(), kP256_PrivateKey_Length * 8,
                                                kSSS_blobType_ELKE_blob) == kStatus_SSS_Success),
                 CHIP_ERROR_INTERNAL);

    mInitialized = true;

exit:
    return CHIP_NO_ERROR;
}

bool PersistentStorageOpKeystoreS200::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(mStorage != nullptr, false);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && (mPendingKeypair != nullptr))
    {
        return true;
    }

    P256SerializedKeypairSSS buf;

    uint16_t keySize = static_cast<uint16_t>(buf.Capacity());
    CHIP_ERROR err =
        mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), buf.Bytes(), keySize);

    return (err == CHIP_NO_ERROR && (keySize == SSS_KEY_PAIR_BLOB_SIZE));
}

CHIP_ERROR PersistentStorageOpKeystoreS200::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                  MutableByteSpan & outCertificateSigningRequest)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Replace previous pending keypair, if any was previously allocated
    ResetPendingKey();

    mPendingKeypair = Platform::New<P256KeypairSSS>();
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);

    mPendingKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA);
    size_t csrLength = outCertificateSigningRequest.size();
    CHIP_ERROR err   = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return err;
    }

    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystoreS200::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                       const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}
CHIP_ERROR PersistentStorageOpKeystoreS200::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    P256SerializedKeypairSSS tmpKeyBlob;
    uint16_t keyBlobLen = tmpKeyBlob.Capacity();

    mPendingKeypair->ExportBlob(tmpKeyBlob);
    ReturnErrorOnFailure(
        mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), tmpKeyBlob.Bytes(), keyBlobLen));

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    ResetPendingKey();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystoreS200::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Remove pending state if matching
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    CHIP_ERROR err = mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName());
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    return err;
}

void PersistentStorageOpKeystoreS200::RevertPendingKeypair()
{
    VerifyOrReturn(mStorage != nullptr);

    // Just reset the pending key, we never stored anything
    ResetPendingKey();
}

CHIP_ERROR PersistentStorageOpKeystoreS200::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                              Crypto::P256ECDSASignature & outSignature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        // We have an override key: sign with it!
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    P256SerializedKeypairSSS keyBlob;
    uint16_t keyBlobLen = keyBlob.Capacity();
    keyBlob.SetLength(keyBlobLen);

    if (fabricIndex != mCachedFabricIndex)
    {
        error =
            mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), keyBlob.Bytes(), keyBlobLen);
        keyBlob.SetLength(keyBlobLen);

        if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            error = CHIP_ERROR_INVALID_FABRIC_INDEX;
        }
        ReturnErrorOnFailure(error);

        if (nullptr == mCachedKeypair)
        {
            mCachedKeypair = Platform::New<P256KeypairSSS>();
            VerifyOrReturnError(mCachedKeypair != nullptr, CHIP_ERROR_NO_MEMORY);
        }

        VerifyOrReturnError(mCachedKeypair->ImportBlob(keyBlob) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    }

    return mCachedKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

Crypto::P256Keypair * PersistentStorageOpKeystoreS200::AllocateEphemeralKeypairForCASE()
{
    // DO NOT CUT AND PASTE without considering the ReleaseEphemeralKeypair().
    // If allocating a derived class, then `ReleaseEphemeralKeypair` MUST
    // de-allocate the derived class after up-casting the base class pointer.
    return Platform::New<Crypto::P256Keypair>();
}

void PersistentStorageOpKeystoreS200::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    // DO NOT CUT AND PASTE without considering the AllocateEphemeralKeypairForCASE().
    // This must delete the same concrete class as allocated in `AllocateEphemeralKeypairForCASE`
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

} // namespace chip
