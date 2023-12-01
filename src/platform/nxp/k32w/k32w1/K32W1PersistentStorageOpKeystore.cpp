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

#include "K32W1PersistentStorageOpKeystore.h"

#include "sss_crypto.h"

namespace chip {

using namespace chip::Crypto;

CHIP_ERROR P256KeypairSSS::Initialize(Crypto::ECPKeyTarget key_target)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    size_t keyBitsLen = kP256_PrivateKey_Length * 8;
    size_t keySize    = SSS_ECP_KEY_SZ(kP256_PrivateKey_Length);

    Clear();

    VerifyOrReturnError(sss_sscp_key_object_init(&mKeyObj, &g_keyStore) == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(sss_sscp_key_object_allocate_handle(
                            &mKeyObj, 0x0u, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P, 3 * kP256_PrivateKey_Length,
                            SSS_KEYPROP_OPERATION_KDF | SSS_KEYPROP_OPERATION_ASYM) == kStatus_SSS_Success,
                        error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(SSS_ECP_GENERATE_KEY(&mKeyObj, keyBitsLen) == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    // The first byte of the public key is the uncompressed marker
    Uint8::to_uchar(mPublicKey)[0] = 0x04;

    // Extract public key, write from the second byte
    VerifyOrExit(SSS_KEY_STORE_GET_PUBKEY(&mKeyObj, Uint8::to_uchar(mPublicKey) + 1, &keySize, &keyBitsLen) == kStatus_SSS_Success,
                 CHIP_ERROR_INTERNAL);

    mInitialized = true;

exit:
    if (mInitialized != true)
        (void) SSS_KEY_OBJ_FREE(&mKeyObj);

    return error;
}

CHIP_ERROR P256KeypairSSS::ExportBlob(P256SerializedKeypairSSS & output) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    size_t keyBlobLen = output.Capacity();
    VerifyOrReturnError(sss_sscp_key_store_export_key(&g_keyStore, &mKeyObj, output.Bytes(), &keyBlobLen,
                                                      kSSS_blobType_ELKE_blob) == kStatus_SSS_Success,
                        CHIP_ERROR_INTERNAL);
    output.SetLength(keyBlobLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256KeypairSSS::ImportBlob(P256SerializedKeypairSSS & input)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (false == mInitialized)
    {
        VerifyOrExit((sss_sscp_key_object_init(&mKeyObj, &g_keyStore) == kStatus_SSS_Success), error = CHIP_ERROR_INTERNAL);

        /* Allocate key handle */
        VerifyOrExit(
            (sss_sscp_key_object_allocate_handle(&mKeyObj, 0x0u, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P,
                                                 3 * kP256_PrivateKey_Length, SSS_KEYPROP_OPERATION_ASYM) == kStatus_SSS_Success),
            error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit((sss_sscp_key_store_import_key(&g_keyStore, &mKeyObj, input.Bytes(), input.Length(), kP256_PrivateKey_Length * 8,
                                                kSSS_blobType_ELKE_blob) == kStatus_SSS_Success),
                 error = CHIP_ERROR_INTERNAL);

    mInitialized = true;

exit:
    return error;
}

CHIP_ERROR P256KeypairSSS::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    sss_sscp_asymmetric_t asyc;
    size_t signatureSize = kP256_ECDSA_Signature_Length_Raw;

    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    VerifyOrExit((sss_sscp_asymmetric_context_init(&asyc, &g_sssSession, &mKeyObj, kAlgorithm_SSS_ECDSA_SHA256, kMode_SSS_Sign) ==
                  kStatus_SSS_Success),
                 error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((sss_sscp_asymmetric_sign_digest(&asyc, digest, kP256_FE_Length, out_signature.Bytes(), &signatureSize) ==
                  kStatus_SSS_Success),
                 error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

exit:
    (void) sss_sscp_asymmetric_context_free(&asyc);
    return error;
}

CHIP_ERROR P256KeypairSSS::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    MutableByteSpan csr(out_csr, csr_length);
    CHIP_ERROR err = GenerateCertificateSigningRequest(this, csr);
    csr_length     = (CHIP_NO_ERROR == err) ? csr.size() : 0;
    return err;
}

void P256KeypairSSS::Clear()
{
    if (mInitialized)
    {
        (void) SSS_KEY_OBJ_FREE(&mKeyObj);
        mInitialized = false;
    }
}

P256KeypairSSS::~P256KeypairSSS()
{
    Clear();
}

bool K32W1PersistentStorageOpKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
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

CHIP_ERROR K32W1PersistentStorageOpKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
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

CHIP_ERROR K32W1PersistentStorageOpKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
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
CHIP_ERROR K32W1PersistentStorageOpKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
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

CHIP_ERROR K32W1PersistentStorageOpKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
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

void K32W1PersistentStorageOpKeystore::RevertPendingKeypair()
{
    VerifyOrReturn(mStorage != nullptr);

    // Just reset the pending key, we never stored anything
    ResetPendingKey();
}

CHIP_ERROR K32W1PersistentStorageOpKeystore::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
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

Crypto::P256Keypair * K32W1PersistentStorageOpKeystore::AllocateEphemeralKeypairForCASE()
{
    // DO NOT CUT AND PASTE without considering the ReleaseEphemeralKeypair().
    // If allocating a derived class, then `ReleaseEphemeralKeypair` MUST
    // de-allocate the derived class after up-casting the base class pointer.
    return Platform::New<Crypto::P256Keypair>();
}

void K32W1PersistentStorageOpKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    // DO NOT CUT AND PASTE without considering the AllocateEphemeralKeypairForCASE().
    // This must delete the same concrete class as allocated in `AllocateEphemeralKeypairForCASE`
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

} // namespace chip
