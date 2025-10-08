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

CHIP_ERROR P256KeypairNXP::ExportBlob(P256SerializedKeypairNXP & output) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    size_t keyBlobLen = output.Capacity();
    auto res          = sss_sscp_key_store_export_key(&g_keyStore, keypair, output.Bytes(), &keyBlobLen, kSSS_blobType_ELKE_blob);
    VerifyOrReturnError(res == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    output.SetLength(keyBlobLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR P256KeypairNXP::ImportBlob(P256SerializedKeypairNXP & input)
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

    P256SerializedKeypairNXP keyBlob;
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
            mCachedKeypair = Platform::New<P256KeypairNXP>();
            VerifyOrReturnError(mCachedKeypair != nullptr, CHIP_ERROR_NO_MEMORY);
        }

        VerifyOrReturnError(mCachedKeypair->ImportBlob(keyBlob) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    }

    return mCachedKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

} // namespace chip
