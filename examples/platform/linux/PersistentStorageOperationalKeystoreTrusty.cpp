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
 *
 *    Copyright 2023 NXP
 */

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

#include "PersistentStorageOperationalKeystoreTrusty.h"

using namespace matter;

namespace chip {
namespace Trusty {

using namespace chip::Crypto;

bool PersistentStorageOperationalKeystoreTrusty::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && (mPendingKeypair != nullptr))
    {
        return true;
    }

    return trusty_matter.HasOpKeypairForFabric(fabricIndex);
}

CHIP_ERROR PersistentStorageOperationalKeystoreTrusty::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                             MutableByteSpan & outCertificateSigningRequest)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Replace previous pending keypair, if any was previously allocated
    ResetPendingKey();

    mPendingKeypair = Platform::New<Crypto::P256Keypair>();
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);

    mPendingKeypair->fabricIndex = fabricIndex;
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

CHIP_ERROR PersistentStorageOperationalKeystoreTrusty::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                                  const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreTrusty::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    int rc = 0;
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Try to store persistent key. On failure, leave everything pending as-is
    rc = trusty_matter.CommitOpKeypairForFabric(mPendingKeypair->p256_handler, fabricIndex);
    if (rc != MATTER_ERROR_OK)
        ReturnErrorOnFailure(CHIP_ERROR_INTERNAL);

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    ResetPendingKey();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreTrusty::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    int rc = 0;
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Remove pending state if matching
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    rc = trusty_matter.RemoveOpKeypairForFabric(fabricIndex);
    if (rc != MATTER_ERROR_OK)
        return CHIP_ERROR_INVALID_FABRIC_INDEX;

    return CHIP_NO_ERROR;
}

void PersistentStorageOperationalKeystoreTrusty::RevertPendingKeypair()
{
    // Just reset the pending key, it hasn't been stored into secure storage.
    ResetPendingKey();
}

CHIP_ERROR PersistentStorageOperationalKeystoreTrusty::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                                         Crypto::P256ECDSASignature & outSignature) const
{
    int rc          = 0;
    size_t sig_size = 0;
    uint8_t sig[kP256_ECDSA_Signature_Length_Raw];

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        // We have an override key: sign with it!
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    rc = trusty_matter.SignWithStoredOpKey(fabricIndex, message.data(), message.size(), sig, sig_size);
    if (rc != MATTER_ERROR_OK)
        return CHIP_ERROR_INTERNAL;

    VerifyOrReturnError(sig_size == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(outSignature.SetLength(sig_size) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    memcpy(outSignature.Bytes(), sig, sig_size);
    return CHIP_NO_ERROR;
}

Crypto::P256Keypair * PersistentStorageOperationalKeystoreTrusty::AllocateEphemeralKeypairForCASE()
{
    return Platform::New<Crypto::P256Keypair>();
}

void PersistentStorageOperationalKeystoreTrusty::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

} // namespace Trusty
} // namespace chip
