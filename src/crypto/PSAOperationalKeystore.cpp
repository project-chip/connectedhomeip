/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "PSAOperationalKeystore.h"
#include "PersistentStorageOperationalKeystore.h"

#include <lib/support/CHIPMem.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

PSAOperationalKeystore::PersistentP256Keypair::PersistentP256Keypair(FabricIndex fabricIndex)
{
    VerifyOrReturn(IsValidFabricIndex(fabricIndex));
    ToPsaContext(mKeypair).key_id = GetPSAKeyAllocator().GetOpKeyId(fabricIndex);
    mInitialized                  = true;
}

PSAOperationalKeystore::PersistentP256Keypair::~PersistentP256Keypair()
{
    // This class requires explicit control of the key lifetime. Therefore, clear the key ID
    // to prevent it from being destroyed by the base class destructor.
    ToPsaContext(mKeypair).key_id = 0;
}

inline psa_key_id_t PSAOperationalKeystore::PersistentP256Keypair::GetKeyId() const
{
    return ToConstPsaContext(mKeypair).key_id;
}

bool PSAOperationalKeystore::PersistentP256Keypair::Exists() const
{
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_status_t status             = psa_get_key_attributes(GetKeyId(), &attributes);

    psa_reset_key_attributes(&attributes);

    return status == PSA_SUCCESS;
}

CHIP_ERROR PSAOperationalKeystore::PersistentP256Keypair::Generate()
{
    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t keyId              = 0;
    size_t publicKeyLength;

    Destroy();

    // Type based on ECC with the elliptic curve SECP256r1 -> PSA_ECC_FAMILY_SECP_R1
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_PERSISTENT);
    psa_set_key_id(&attributes, GetKeyId());
    GetPSAKeyAllocator().UpdateKeyAttributes(attributes);

    status = psa_generate_key(&attributes, &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_export_public_key(keyId, mPublicKey.Bytes(), mPublicKey.Length(), &publicKeyLength);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(publicKeyLength == kP256_PublicKey_Length, error = CHIP_ERROR_INTERNAL);

exit:
    psa_reset_key_attributes(&attributes);

    return error;
}

CHIP_ERROR PSAOperationalKeystore::PersistentP256Keypair::Destroy()
{
    psa_status_t status = psa_destroy_key(GetKeyId());

    VerifyOrReturnError(status != PSA_ERROR_INVALID_HANDLE, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

bool PSAOperationalKeystore::HasPendingOpKeypair() const
{
    return mPendingFabricIndex != kUndefinedFabricIndex;
}

bool PSAOperationalKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    if (mPendingFabricIndex == fabricIndex)
    {
        return mIsPendingKeypairActive;
    }

    return PersistentP256Keypair(fabricIndex).Exists();
}

CHIP_ERROR PSAOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (HasPendingOpKeypair())
    {
        VerifyOrReturnError(fabricIndex == mPendingFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    }

    if (mPendingKeypair == nullptr)
    {
        mPendingKeypair = Platform::New<PersistentP256Keypair>(fabricIndex);
    }

    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(mPendingKeypair->Generate());

    size_t csrLength = outCertificateSigningRequest.size();
    ReturnErrorOnFailure(mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength));
    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSAOperationalKeystore::PersistentP256Keypair::Deserialize(P256SerializedKeypair & input)
{
    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t keyId              = 0;
    VerifyOrReturnError(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, CHIP_ERROR_INVALID_ARGUMENT);

    Destroy();

    // Type based on ECC with the elliptic curve SECP256r1 -> PSA_ECC_FAMILY_SECP_R1
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_PERSISTENT);
    psa_set_key_id(&attributes, GetKeyId());
    GetPSAKeyAllocator().UpdateKeyAttributes(attributes);

    status = psa_import_key(&attributes, input.ConstBytes() + mPublicKey.Length(), kP256_PrivateKey_Length, &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    memcpy(mPublicKey.Bytes(), input.ConstBytes(), mPublicKey.Length());

exit:
    LogPsaError(status);
    psa_reset_key_attributes(&attributes);

    return error;
}

CHIP_ERROR PSAOperationalKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && mPendingFabricIndex == fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);
    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSAOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && mPendingFabricIndex == fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive, CHIP_ERROR_INCORRECT_STATE);

    ReleasePendingKeypair();

    return CHIP_NO_ERROR;
}

CHIP_ERROR PSAOperationalKeystore::ExportOpKeypairForFabric(FabricIndex fabricIndex, Crypto::P256SerializedKeypair & outKeypair)
{
    // Currently exporting the key is forbidden in PSAOperationalKeystore because the PSA_KEY_USAGE_EXPORT usage flag is not set, so
    // there is no need to compile the code for the device, but there should be an implementation for test purposes to verify if
    // the psa_export_key returns an error.
#if CHIP_CONFIG_TEST
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(HasOpKeypairForFabric(fabricIndex), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    size_t outSize = 0;
    psa_status_t status =
        psa_export_key(PersistentP256Keypair(fabricIndex).GetKeyId(), outKeypair.Bytes(), outKeypair.Capacity(), &outSize);

    if (status == PSA_ERROR_BUFFER_TOO_SMALL)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else if (status == PSA_ERROR_NOT_PERMITTED)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    else if (status != PSA_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    outKeypair.SetLength(outSize);

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR PSAOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mPendingFabricIndex == fabricIndex)
    {
        RevertPendingKeypair();
        return CHIP_NO_ERROR;
    }

    return PersistentP256Keypair(fabricIndex).Destroy();
}

void PSAOperationalKeystore::RevertPendingKeypair()
{
    VerifyOrReturn(HasPendingOpKeypair());
    mPendingKeypair->Destroy();
    ReleasePendingKeypair();
}

CHIP_ERROR PSAOperationalKeystore::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                     Crypto::P256ECDSASignature & outSignature) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mPendingFabricIndex == fabricIndex)
    {
        VerifyOrReturnError(mIsPendingKeypairActive, CHIP_ERROR_INVALID_FABRIC_INDEX);
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    PersistentP256Keypair keypair(fabricIndex);
    VerifyOrReturnError(keypair.Exists(), CHIP_ERROR_INVALID_FABRIC_INDEX);

    return keypair.ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

Crypto::P256Keypair * PSAOperationalKeystore::AllocateEphemeralKeypairForCASE()
{
    return Platform::New<Crypto::P256Keypair>();
}

void PSAOperationalKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete(keypair);
}

void PSAOperationalKeystore::ReleasePendingKeypair()
{
    Platform::Delete(mPendingKeypair);
    mPendingKeypair         = nullptr;
    mPendingFabricIndex     = kUndefinedFabricIndex;
    mIsPendingKeypairActive = false;
}

CHIP_ERROR PSAOperationalKeystore::MigrateOpKeypairForFabric(FabricIndex fabricIndex,
                                                             OperationalKeystore & operationalKeystore) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    P256SerializedKeypair serializedKeypair;

    // Do not allow overwriting the existing key and just remove it from the previous Operational Keystore if needed.
    if (!HasOpKeypairForFabric(fabricIndex))
    {
        ReturnErrorOnFailure(operationalKeystore.ExportOpKeypairForFabric(fabricIndex, serializedKeypair));

        PersistentP256Keypair keypair(fabricIndex);
        ReturnErrorOnFailure(keypair.Deserialize(serializedKeypair));

        // Migrated key is not useful anymore, remove it from the previous keystore.
        ReturnErrorOnFailure(operationalKeystore.RemoveOpKeypairForFabric(fabricIndex));
    }
    else if (operationalKeystore.HasOpKeypairForFabric(fabricIndex))
    {
        ReturnErrorOnFailure(operationalKeystore.RemoveOpKeypairForFabric(fabricIndex));
    }

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
