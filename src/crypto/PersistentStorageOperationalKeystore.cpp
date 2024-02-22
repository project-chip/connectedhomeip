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

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

#include "PersistentStorageOperationalKeystore.h"

namespace chip {

using namespace chip::Crypto;

namespace {

// Tags for our operational keypair storage.
constexpr TLV::Tag kOpKeyVersionTag = TLV::ContextTag(0);
constexpr TLV::Tag kOpKeyDataTag    = TLV::ContextTag(1);

// If this version grows beyond UINT16_MAX, adjust OpKeypairTLVMaxSize
// accordingly.
constexpr uint16_t kOpKeyVersion = 1;

constexpr size_t OpKeyTLVMaxSize()
{
    // Version and serialized key
    return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
}

/** WARNING: This can leave the operational key on the stack somewhere, since many of the platform
 *           APIs use stack buffers and do not sanitize! This implementation is for example purposes
 *           only of the API and it is recommended to avoid directly accessing raw private key bits
 *           in storage.
 */
CHIP_ERROR StoreOperationalKey(FabricIndex fabricIndex, PersistentStorageDelegate * storage, P256Keypair * keypair)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (storage != nullptr) && (keypair != nullptr),
                        CHIP_ERROR_INVALID_ARGUMENT);

    // Use a SensitiveDataBuffer to get RAII secret data clearing on scope exit.
    Crypto::SensitiveDataBuffer<OpKeyTLVMaxSize()> buf;
    TLV::TLVWriter writer;

    writer.Init(buf.Bytes(), buf.Capacity());

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

    ReturnErrorOnFailure(writer.Put(kOpKeyVersionTag, kOpKeyVersion));

    {
        // P256SerializedKeypair has RAII secret clearing
        Crypto::P256SerializedKeypair serializedOpKey;
        ReturnErrorOnFailure(keypair->Serialize(serializedOpKey));

        ReturnErrorOnFailure(writer.Put(kOpKeyDataTag, ByteSpan(serializedOpKey.Bytes(), serializedOpKey.Length())));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto opKeyLength = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(opKeyLength), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(storage->SyncSetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), buf.ConstBytes(),
                                                  static_cast<uint16_t>(opKeyLength)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExportStoredOpKey(FabricIndex fabricIndex, PersistentStorageDelegate * storage,
                             Crypto::P256SerializedKeypair & serializedOpKey)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Use a SensitiveDataBuffer to get RAII secret data clearing on scope exit.
    Crypto::SensitiveDataBuffer<OpKeyTLVMaxSize()> buf;

    // Load up the operational key structure from storage
    uint16_t size = static_cast<uint16_t>(buf.Capacity());
    ReturnErrorOnFailure(
        storage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), buf.Bytes(), size));

    buf.SetLength(static_cast<size_t>(size));

    // Read-out the operational key TLV entry.
    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.Bytes(), buf.Length());

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ReturnErrorOnFailure(reader.Next(kOpKeyVersionTag));
    uint16_t opKeyVersion;
    ReturnErrorOnFailure(reader.Get(opKeyVersion));
    VerifyOrReturnError(opKeyVersion == kOpKeyVersion, CHIP_ERROR_VERSION_MISMATCH);

    ReturnErrorOnFailure(reader.Next(kOpKeyDataTag));
    {
        ByteSpan keyData;
        ReturnErrorOnFailure(reader.GetByteView(keyData));

        // Unfortunately, we have to copy the data into a P256SerializedKeypair.
        VerifyOrReturnError(keyData.size() <= serializedOpKey.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

        ReturnErrorOnFailure(reader.ExitContainer(containerType));

        memcpy(serializedOpKey.Bytes(), keyData.data(), keyData.size());
        serializedOpKey.SetLength(keyData.size());
    }

    return CHIP_NO_ERROR;
}

/** WARNING: This can leave the operational key on the stack somewhere, since many of the platform
 *           APIs use stack buffers and do not sanitize! This implementation is for example purposes
 *           only of the API and it is recommended to avoid directly accessing raw private key bits
 *           in storage.
 */
CHIP_ERROR SignWithStoredOpKey(FabricIndex fabricIndex, PersistentStorageDelegate * storage, const ByteSpan & message,
                               P256ECDSASignature & outSignature)
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (storage != nullptr), CHIP_ERROR_INVALID_ARGUMENT);

    // Use RAII scoping for the transient keypair, to make sure it doesn't get leaked on any error paths.
    // Key is put in heap since signature is a costly stack operation and P256Keypair is
    // a costly class depending on the backend.
    auto transientOperationalKeypair = Platform::MakeUnique<P256Keypair>();
    if (!transientOperationalKeypair)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Scope 1: Load up the keypair data from storage
    P256SerializedKeypair serializedOpKey;
    CHIP_ERROR err = ExportStoredOpKey(fabricIndex, storage, serializedOpKey);
    if (CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err)
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    // Load-up key material
    // WARNING: This makes use of the raw key bits
    ReturnErrorOnFailure(transientOperationalKeypair->Deserialize(serializedOpKey));

    // Scope 2: Sign message with the keypair
    return transientOperationalKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

} // namespace

bool PersistentStorageOperationalKeystore::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(mStorage != nullptr, false);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && (mPendingKeypair != nullptr))
    {
        return true;
    }

    // TODO(#16958): need to actually read the key to know if it's there due to platforms not
    //               properly enforcing CHIP_ERROR_BUFFER_TOO_SMALL behavior needed by
    //               PersistentStorageDelegate. Very unfortunate, needs fixing ASAP.

    // Use a SensitiveDataBuffer to get RAII secret data clearing on scope exit.
    Crypto::SensitiveDataBuffer<OpKeyTLVMaxSize()> buf;

    uint16_t keySize = static_cast<uint16_t>(buf.Capacity());
    CHIP_ERROR err =
        mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName(), buf.Bytes(), keySize);

    return (err == CHIP_NO_ERROR);
}

CHIP_ERROR PersistentStorageOperationalKeystore::NewOpKeypairForFabric(FabricIndex fabricIndex,
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

    mPendingKeypair = Platform::New<Crypto::P256Keypair>();
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

CHIP_ERROR PersistentStorageOperationalKeystore::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
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

CHIP_ERROR PersistentStorageOperationalKeystore::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Try to store persistent key. On failure, leave everything pending as-is
    CHIP_ERROR err = StoreOperationalKey(fabricIndex, mStorage, mPendingKeypair);
    ReturnErrorOnFailure(err);

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    ResetPendingKey();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystore::ExportOpKeypairForFabric(FabricIndex fabricIndex,
                                                                          Crypto::P256SerializedKeypair & outKeypair)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return ExportStoredOpKey(fabricIndex, mStorage, outKeypair);
}

CHIP_ERROR PersistentStorageOperationalKeystore::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
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

void PersistentStorageOperationalKeystore::RevertPendingKeypair()
{
    VerifyOrReturn(mStorage != nullptr);

    // Just reset the pending key, we never stored anything
    ResetPendingKey();
}

CHIP_ERROR PersistentStorageOperationalKeystore::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                                   Crypto::P256ECDSASignature & outSignature) const
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        // We have an override key: sign with it!
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    return SignWithStoredOpKey(fabricIndex, mStorage, message, outSignature);
}

Crypto::P256Keypair * PersistentStorageOperationalKeystore::AllocateEphemeralKeypairForCASE()
{
    // DO NOT CUT AND PASTE without considering the ReleaseEphemeralKeypair().
    // If allocating a derived class, then `ReleaseEphemeralKeypair` MUST
    // de-allocate the derived class after up-casting the base class pointer.
    return Platform::New<Crypto::P256Keypair>();
}

void PersistentStorageOperationalKeystore::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    // DO NOT CUT AND PASTE without considering the AllocateEphemeralKeypairForCASE().
    // This must delete the same concrete class as allocated in `AllocateEphemeralKeypairForCASE`
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

CHIP_ERROR PersistentStorageOperationalKeystore::MigrateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                           OperationalKeystore & operationalKeystore) const
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    P256SerializedKeypair serializedKeypair;

    // Do not allow overwriting the existing key and just remove it from the previous Operational Keystore if needed.
    if (!HasOpKeypairForFabric(fabricIndex))
    {
        ReturnErrorOnFailure(operationalKeystore.ExportOpKeypairForFabric(fabricIndex, serializedKeypair));

        auto operationalKeypair = Platform::MakeUnique<P256Keypair>();
        if (!operationalKeypair)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        ReturnErrorOnFailure(operationalKeypair->Deserialize(serializedKeypair));
        ReturnErrorOnFailure(StoreOperationalKey(fabricIndex, mStorage, operationalKeypair.get()));

        ReturnErrorOnFailure(operationalKeystore.RemoveOpKeypairForFabric(fabricIndex));
    }
    else if (operationalKeystore.HasOpKeypairForFabric(fabricIndex))
    {
        ReturnErrorOnFailure(operationalKeystore.RemoveOpKeypairForFabric(fabricIndex));
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
