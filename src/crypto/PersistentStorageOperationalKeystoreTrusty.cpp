/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <trusty_matter.h>

#include "PersistentStorageOperationalKeystore.h"

using namespace matter;
using namespace chip::Crypto;

namespace chip {

TrustyMatter & GetTrustyMatter()
{
    static TrustyMatter instance;
    return instance;
}

bool PersistentStorageOperationalKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && (mPendingKeypair != nullptr))
    {
        return true;
    }

    return GetTrustyMatter().HasOpKeypairForFabric(fabricIndex);
}

CHIP_ERROR PersistentStorageOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                       MutableByteSpan & outCertificateSigningRequest)
{
    P256SerializedKeypair serializedKeypair;
    uint8_t public_key[kP256_PublicKey_Length] = { 0 };
    uint64_t p256_handle                       = 0;

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

    memcpy(serializedKeypair.Bytes(), public_key, kP256_PublicKey_Length);

    // kTrustyMagicNumberFabricIndex = { 0x2, 0x3, 0x5, 0x7, 0x0B, 0x0D, 0x11 }
    // It is a flag to inform the Trusty OS to use a fixed p256_handle, and provide an opaque key
    memcpy(&p256_handle, kTrustyMagicNumberFabricIndex, sizeof(kTrustyMagicNumberFabricIndex));
    // FabricIndex is a uint8_t indeed
    static_assert(1 == sizeof(FabricIndex));

    // Append the fabric index after the magic number
    memcpy((uint8_t *) &p256_handle + sizeof(kTrustyMagicNumberFabricIndex), &fabricIndex, sizeof(FabricIndex));

    memcpy(serializedKeypair.Bytes() + kP256_PublicKey_Length, &p256_handle, sizeof(uint64_t));
    serializedKeypair.SetLength(kP256_PublicKey_Length + sizeof(uint64_t));

    ReturnErrorOnFailure(mPendingKeypair->Deserialize(serializedKeypair));

    mPendingKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA);

    size_t csrLength = outCertificateSigningRequest.size();
    CHIP_ERROR err   = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return CHIP_ERROR_HSM;
    }

    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                            const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    P256SerializedKeypair serializedOpKey;
    uint64_t p256_handle          = 0;
    uint8_t p256_handle_expect[8] = { 0 };

    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Try to store persistent key. On failure, leave everything pending as-is
    CHIP_ERROR err = mPendingKeypair->Serialize(serializedOpKey);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    memcpy(&p256_handle, serializedOpKey.Bytes() + kP256_PublicKey_Length, sizeof(uint64_t));

    memcpy(&p256_handle_expect[0], kTrustyMagicNumberFabricIndex, sizeof(kTrustyMagicNumberFabricIndex));
    memcpy(&p256_handle_expect[7], &fabricIndex, sizeof(FabricIndex));
    VerifyOrReturnError(!memcmp(&p256_handle, p256_handle_expect, sizeof(p256_handle_expect)), CHIP_ERROR_INTERNAL);

    int rc = GetTrustyMatter().CommitOpKeypairForFabric(p256_handle, fabricIndex);
    VerifyOrReturnError(rc == MATTER_ERROR_OK, CHIP_ERROR_INTERNAL);

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    ResetPendingKey();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::ExportOpKeypairForFabric(FabricIndex fabricIndex,
                                                                          Crypto::P256SerializedKeypair & outKeypair)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PersistentStorageOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    int rc = 0;
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Remove pending state if matching
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    rc = GetTrustyMatter().RemoveOpKeypairForFabric(fabricIndex);
    if (rc != MATTER_ERROR_OK)
        return CHIP_ERROR_INTERNAL;

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

    rc = GetTrustyMatter().SignWithStoredOpKey(fabricIndex, message.data(), message.size(), sig, sig_size);
    if (rc != MATTER_ERROR_OK)
        return CHIP_ERROR_INTERNAL;

    VerifyOrReturnError(sig_size == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(outSignature.SetLength(sig_size) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    memcpy(outSignature.Bytes(), sig, sig_size);
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
