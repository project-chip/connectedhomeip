/*
 *
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

#include "CustomCSRResponseOperationalKeyStore.h"

#include <credentials/FabricTable.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

namespace {
// Tags for our operational keypair storage.
constexpr TLV::Tag kOpKeyVersionTag = TLV::ContextTag(0);
constexpr TLV::Tag kOpKeyDataTag    = TLV::ContextTag(1);

constexpr size_t OpKeyTLVMaxSize()
{
    // Version and serialized key
    return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
}
} // namespace

CHIP_ERROR CustomCSRResponseOperationalKeyStore::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                       MutableByteSpan & outCertificateSigningRequest)
{
    if (fabricIndex == 1)
    {
        return PersistentStorageOperationalKeystore::NewOpKeypairForFabric(fabricIndex, outCertificateSigningRequest);
    }

    return ReuseOpKeypair(fabricIndex, outCertificateSigningRequest);
}

CHIP_ERROR CustomCSRResponseOperationalKeyStore::ReuseOpKeypair(FabricIndex fabricIndex, MutableByteSpan & outCSR)
{
    //
    // DO NOT COPY THIS METHOD - IT IS FOR TESTING PURPOSES ONLY
    //

    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Replace previous pending keypair, if any was previously allocated
    ResetPendingKey();

    mPendingKeypair = Platform::New<Crypto::P256Keypair>();
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);

    // Scope 1: Load up the keypair data from storage
    {
        // Use a CapacityBoundBuffer to get RAII secret data clearing on scope exit.
        Crypto::CapacityBoundBuffer<OpKeyTLVMaxSize()> buf;

        // Load up the operational key structure from storage
        uint16_t size = static_cast<uint16_t>(buf.Capacity());

        // In order to retrieve a keypair that has already been registered, assume the device
        // as already been commissioned and fabric index 1 is the registered fabric.
        CHIP_ERROR err =
            mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::FabricOpKey(1 /* fabricIndex */).KeyName(), buf.Bytes(), size);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            err = CHIP_ERROR_INVALID_FABRIC_INDEX;
        }
        ReturnErrorOnFailure(err);
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

        ReturnErrorOnFailure(reader.Next(kOpKeyDataTag));
        {
            ByteSpan keyData;
            Crypto::P256SerializedKeypair serializedOpKey;
            ReturnErrorOnFailure(reader.GetByteView(keyData));

            // Unfortunately, we have to copy the data into a P256SerializedKeypair.
            VerifyOrReturnError(keyData.size() <= serializedOpKey.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

            // Before doing anything with the key, validate format further.
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
            ReturnErrorOnFailure(reader.VerifyEndOfContainer());

            memcpy(serializedOpKey.Bytes(), keyData.data(), keyData.size());
            serializedOpKey.SetLength(keyData.size());

            // Load-up key material
            // WARNING: This makes use of the raw key bits
            ReturnErrorOnFailure(mPendingKeypair->Deserialize(serializedOpKey));
        }
    }

    size_t outCSRLength = outCSR.size();
    CHIP_ERROR err      = mPendingKeypair->NewCertificateSigningRequest(outCSR.data(), outCSRLength);
    if (CHIP_NO_ERROR != err)
    {
        ResetPendingKey();
        return err;
    }

    outCSR.reduce_size(outCSRLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

} // namespace chip
