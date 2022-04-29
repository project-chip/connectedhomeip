/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

/**
 * @brief Defines a table of fabrics that have provisioned the device.
 */

#include "FabricTable.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

namespace chip {
using namespace Credentials;
using namespace Crypto;

CHIP_ERROR FabricInfo::SetFabricLabel(const CharSpan & fabricLabel)
{
    Platform::CopyString(mFabricLabel, fabricLabel);

    return CHIP_NO_ERROR;
}

namespace {
// Tags for our metadata storage.
constexpr TLV::Tag kVendorIdTag    = TLV::ContextTag(0);
constexpr TLV::Tag kFabricLabelTag = TLV::ContextTag(1);
constexpr TLV::Tag kEventNumberTag = TLV::ContextTag(2);

// Tags for our operational keypair storage.
constexpr TLV::Tag kOpKeyVersionTag = TLV::ContextTag(0);
constexpr TLV::Tag kOpKeyDataTag    = TLV::ContextTag(1);

// If this version grows beyond UINT16_MAX, adjust OpKeypairTLVMaxSize
// accordingly.
constexpr uint16_t kOpKeyVersion = 1;

// Tags for our index list storage.
constexpr TLV::Tag kNextAvailableFabricIndexTag = TLV::ContextTag(0);
constexpr TLV::Tag kFabricIndicesTag            = TLV::ContextTag(1);
} // anonymous namespace

CHIP_ERROR FabricInfo::CommitToStorage(PersistentStorageDelegate * storage)
{
    DefaultStorageKeyAllocator keyAlloc;

    VerifyOrReturnError(mRootCert.size() <= kMaxCHIPCertLength && mICACert.size() <= kMaxCHIPCertLength &&
                            mNOCCert.size() <= kMaxCHIPCertLength,
                        CHIP_ERROR_BUFFER_TOO_SMALL);
    static_assert(kMaxCHIPCertLength <= UINT16_MAX, "Casting to uint16_t won't be safe");

    ReturnErrorOnFailure(
        storage->SyncSetKeyValue(keyAlloc.FabricRCAC(mFabricIndex), mRootCert.data(), static_cast<uint16_t>(mRootCert.size())));

    // Workaround for the fact that some storage backends do not allow storing
    // a nullptr with 0 length.  See
    // https://github.com/project-chip/connectedhomeip/issues/16030.
    if (!mICACert.empty())
    {
        ReturnErrorOnFailure(
            storage->SyncSetKeyValue(keyAlloc.FabricICAC(mFabricIndex), mICACert.data(), static_cast<uint16_t>(mICACert.size())));
    }
    else
    {
        // Make sure there is no stale data.
        CHIP_ERROR err = storage->SyncDeleteKeyValue(keyAlloc.FabricICAC(mFabricIndex));
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ReturnErrorOnFailure(err);
        }
    }

    ReturnErrorOnFailure(
        storage->SyncSetKeyValue(keyAlloc.FabricNOC(mFabricIndex), mNOCCert.data(), static_cast<uint16_t>(mNOCCert.size())));

    {
        Crypto::P256SerializedKeypair serializedOpKey;
        if (mOperationalKey != nullptr)
        {
            ReturnErrorOnFailure(mOperationalKey->Serialize(serializedOpKey));
        }
        else
        {
            // Could we just not store it instead?  What would deserialize need
            // to do then?
            P256Keypair keypair;
            ReturnErrorOnFailure(keypair.Initialize());
            ReturnErrorOnFailure(keypair.Serialize(serializedOpKey));
        }

        uint8_t buf[OpKeyTLVMaxSize()];
        TLV::TLVWriter writer;
        writer.Init(buf);

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

        ReturnErrorOnFailure(writer.Put(kOpKeyVersionTag, kOpKeyVersion));

        ReturnErrorOnFailure(writer.Put(kOpKeyDataTag, ByteSpan(serializedOpKey.Bytes(), serializedOpKey.Length())));

        ReturnErrorOnFailure(writer.EndContainer(outerType));

        const auto opKeyLength = writer.GetLengthWritten();
        VerifyOrReturnError(CanCastTo<uint16_t>(opKeyLength), CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(storage->SyncSetKeyValue(keyAlloc.FabricOpKey(mFabricIndex), buf, static_cast<uint16_t>(opKeyLength)));
    }

    {
        uint8_t buf[MetadataTLVMaxSize()];
        TLV::TLVWriter writer;
        writer.Init(buf);

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

        ReturnErrorOnFailure(writer.Put(kVendorIdTag, mVendorId));

        ReturnErrorOnFailure(writer.PutString(kFabricLabelTag, CharSpan::fromCharString(mFabricLabel)));

        uint16_t size = sizeof(mEventNumber);
        if (storage->SyncGetKeyValue(keyAlloc.IMEventNumber(), &mEventNumber, size) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            mEventNumber = 0;
        }

        ReturnErrorOnFailure(writer.EndContainer(outerType));

        const auto metadataLength = writer.GetLengthWritten();
        VerifyOrReturnError(CanCastTo<uint16_t>(metadataLength), CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(
            storage->SyncSetKeyValue(keyAlloc.FabricMetadata(mFabricIndex), buf, static_cast<uint16_t>(metadataLength)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::LoadFromStorage(PersistentStorageDelegate * storage)
{
    DefaultStorageKeyAllocator keyAlloc;

    ChipLogProgress(Inet, "Loading from storage for fabric index 0x%x", static_cast<unsigned>(mFabricIndex));

    // Scopes for "size" so we don't forget to re-initialize it between gets,
    // since each get modifies it.
    {
        uint8_t buf[Credentials::kMaxCHIPCertLength];
        uint16_t size = sizeof(buf);
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricRCAC(mFabricIndex), buf, size));
        ReturnErrorOnFailure(SetRootCert(ByteSpan(buf, size)));
    }

    {
        uint8_t buf[Credentials::kMaxCHIPCertLength];
        uint16_t size  = sizeof(buf);
        CHIP_ERROR err = storage->SyncGetKeyValue(keyAlloc.FabricICAC(mFabricIndex), buf, size);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // That's OK; that just means no ICAC.
            size = 0;
        }
        else
        {
            ReturnErrorOnFailure(err);
        }
        ReturnErrorOnFailure(SetICACert(ByteSpan(buf, size)));
    }

    {
        uint8_t buf[Credentials::kMaxCHIPCertLength];
        uint16_t size = sizeof(buf);
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricNOC(mFabricIndex), buf, size));
        ByteSpan nocCert(buf, size);
        NodeId nodeId;
        ReturnErrorOnFailure(ExtractNodeIdFabricIdFromOpCert(nocCert, &nodeId, &mFabricId));
        // The compressed fabric ID doesn't change for a fabric over time.
        // Computing it here will save computational overhead when it's accessed by other
        // parts of the code.
        ReturnErrorOnFailure(GeneratePeerId(mFabricId, nodeId, &mOperationalId));
        ReturnErrorOnFailure(SetNOCCert(nocCert));
    }

    {
        // Use a CapacityBoundBuffer to get RAII secret data clearing on scope exit.
        Crypto::CapacityBoundBuffer<OpKeyTLVMaxSize()> buf;
        uint16_t size = static_cast<uint16_t>(buf.Capacity());
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricOpKey(mFabricIndex), buf.Bytes(), size));
        buf.SetLength(static_cast<size_t>(size));

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
        ByteSpan keyData;
        ReturnErrorOnFailure(reader.GetByteView(keyData));

        // Unfortunately, we have to copy the data into a P256SerializedKeypair.
        Crypto::P256SerializedKeypair serializedOpKey;
        VerifyOrReturnError(keyData.size() <= serializedOpKey.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

        memcpy(serializedOpKey.Bytes(), keyData.data(), keyData.size());
        serializedOpKey.SetLength(keyData.size());

        if (mOperationalKey == nullptr)
        {
#ifdef ENABLE_HSM_CASE_OPS_KEY
            mOperationalKey = chip::Platform::New<P256KeypairHSM>();
#else
            mOperationalKey = chip::Platform::New<P256Keypair>();
#endif
        }
        VerifyOrReturnError(mOperationalKey != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(mOperationalKey->Deserialize(serializedOpKey));

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        ReturnErrorOnFailure(reader.VerifyEndOfContainer());
    }

    {
        uint8_t buf[MetadataTLVMaxSize()];
        uint16_t size = sizeof(buf);
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricMetadata(mFabricIndex), buf, size));
        TLV::ContiguousBufferTLVReader reader;
        reader.Init(buf, size);

        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        TLV::TLVType containerType;
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        ReturnErrorOnFailure(reader.Next(kVendorIdTag));
        ReturnErrorOnFailure(reader.Get(mVendorId));

        ReturnErrorOnFailure(reader.Next(kFabricLabelTag));
        CharSpan label;
        ReturnErrorOnFailure(reader.Get(label));

        VerifyOrReturnError(label.size() <= kFabricLabelMaxLengthInBytes, CHIP_ERROR_BUFFER_TOO_SMALL);
        Platform::CopyString(mFabricLabel, label);

        ReturnErrorOnFailure(reader.Next(kEventNumberTag));
        ReturnErrorOnFailure(reader.Get(mEventNumber));

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        ReturnErrorOnFailure(reader.VerifyEndOfContainer());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::GeneratePeerId(FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId) const
{
    ReturnErrorCodeIf(compressedPeerId == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    uint8_t compressedFabricIdBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricIdSpan(compressedFabricIdBuf);
    P256PublicKey rootPubkey;

    {
        P256PublicKeySpan rootPubkeySpan;
        ReturnErrorOnFailure(GetRootPubkey(rootPubkeySpan));
        rootPubkey = rootPubkeySpan;
    }

    ReturnErrorOnFailure(GenerateCompressedFabricId(rootPubkey, fabricId, compressedFabricIdSpan));

    // Decode compressed fabric ID accounting for endianness, as GenerateCompressedFabricId()
    // returns a binary buffer and is agnostic of usage of the output as an integer type.
    CompressedFabricId compressedFabricId = Encoding::BigEndian::Get64(compressedFabricIdBuf);
    compressedPeerId->SetCompressedFabricId(compressedFabricId);
    compressedPeerId->SetNodeId(nodeId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::DeleteFromStorage(PersistentStorageDelegate * storage, FabricIndex fabricIndex)
{
    DefaultStorageKeyAllocator keyAlloc;

    // Try to delete all the state even if one of the deletes fails.
    typedef const char * (DefaultStorageKeyAllocator::*KeyGetter)(FabricIndex);
    constexpr KeyGetter keyGetters[] = { &DefaultStorageKeyAllocator::FabricNOC, &DefaultStorageKeyAllocator::FabricICAC,
                                         &DefaultStorageKeyAllocator::FabricRCAC, &DefaultStorageKeyAllocator::FabricMetadata,
                                         &DefaultStorageKeyAllocator::FabricOpKey };

    CHIP_ERROR prevDeleteErr = CHIP_NO_ERROR;

    for (auto & keyGetter : keyGetters)
    {
        CHIP_ERROR deleteErr = storage->SyncDeleteKeyValue((keyAlloc.*keyGetter)(fabricIndex));
        // Keys not existing is not really an error condition.
        if (prevDeleteErr == CHIP_NO_ERROR && deleteErr != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            prevDeleteErr = deleteErr;
        }
    }
    if (prevDeleteErr != CHIP_NO_ERROR)
    {
        ChipLogDetail(Discovery, "Error deleting part of fabric %d: %" CHIP_ERROR_FORMAT, fabricIndex, prevDeleteErr.Format());
    }
    return prevDeleteErr;
}

CHIP_ERROR FabricInfo::SetOperationalKeypair(const P256Keypair * keyPair)
{
    VerifyOrReturnError(keyPair != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(keyPair->Serialize(serialized));
    if (mOperationalKey == nullptr)
    {
#ifdef ENABLE_HSM_CASE_OPS_KEY
        mOperationalKey = chip::Platform::New<P256KeypairHSM>();
#else
        mOperationalKey = chip::Platform::New<P256Keypair>();
#endif
    }
    VerifyOrReturnError(mOperationalKey != nullptr, CHIP_ERROR_NO_MEMORY);
    return mOperationalKey->Deserialize(serialized);
}

void FabricInfo::ReleaseCert(MutableByteSpan & cert)
{
    if (cert.data() != nullptr)
    {
        chip::Platform::MemoryFree(cert.data());
    }
    cert = MutableByteSpan();
}

CHIP_ERROR FabricInfo::SetCert(MutableByteSpan & dstCert, const ByteSpan & srcCert)
{
    ReleaseCert(dstCert);
    if (srcCert.data() == nullptr || srcCert.size() == 0)
    {
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(srcCert.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(srcCert.size()), CHIP_ERROR_INVALID_ARGUMENT);

    dstCert = MutableByteSpan(static_cast<uint8_t *>(chip::Platform::MemoryAlloc(srcCert.size())), srcCert.size());
    VerifyOrReturnError(dstCert.data() != nullptr, CHIP_ERROR_NO_MEMORY);

    memcpy(dstCert.data(), srcCert.data(), srcCert.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, ValidationContext & context,
                                         PeerId & nocPeerId, FabricId & fabricId, Crypto::P256PublicKey & nocPubkey) const
{
    // TODO - Optimize credentials verification logic
    //        The certificate chain construction and verification is a compute and memory intensive operation.
    //        It can be optimized by not loading certificate (i.e. rcac) that's local and implicitly trusted.
    //        The FindValidCert() algorithm will need updates to achieve this refactor.
    constexpr uint8_t kMaxNumCertsInOpCreds = 3;

    ChipCertificateSet certificates;
    ReturnErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds));

    ReturnErrorOnFailure(certificates.LoadCert(mRootCert, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    if (!icac.empty())
    {
        ReturnErrorOnFailure(certificates.LoadCert(icac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));
    }

    ReturnErrorOnFailure(certificates.LoadCert(noc, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    const ChipDN & nocSubjectDN              = certificates.GetLastCert()[0].mSubjectDN;
    const CertificateKeyId & nocSubjectKeyId = certificates.GetLastCert()[0].mSubjectKeyId;

    const ChipCertificateData * resultCert = nullptr;
    // FindValidCert() checks the certificate set constructed by loading noc, icac and mRootCert.
    // It confirms that the certs link correctly (noc -> icac -> mRootCert), and have been correctly signed.
    ReturnErrorOnFailure(certificates.FindValidCert(nocSubjectDN, nocSubjectKeyId, context, &resultCert));

    NodeId nodeId;
    ReturnErrorOnFailure(ExtractNodeIdFabricIdFromOpCert(certificates.GetLastCert()[0], &nodeId, &fabricId));

    CHIP_ERROR err;
    FabricId icacFabricId = kUndefinedFabricId;
    if (!icac.empty())
    {
        err = ExtractFabricIdFromCert(certificates.GetCertSet()[1], &icacFabricId);
        if (err == CHIP_NO_ERROR)
        {
            ReturnErrorCodeIf(icacFabricId != fabricId, CHIP_ERROR_FABRIC_MISMATCH_ON_ICA);
        }
        // FabricId is optional field in ICAC and "not found" code is not treated as error.
        else if (err != CHIP_ERROR_NOT_FOUND)
        {
            return err;
        }
    }

    FabricId rcacFabricId = kUndefinedFabricId;
    err                   = ExtractFabricIdFromCert(certificates.GetCertSet()[0], &rcacFabricId);
    if (err == CHIP_NO_ERROR)
    {
        ReturnErrorCodeIf(rcacFabricId != fabricId, CHIP_ERROR_WRONG_CERT_DN);
    }
    // FabricId is optional field in RCAC and "not found" code is not treated as error.
    else if (err != CHIP_ERROR_NOT_FOUND)
    {
        return err;
    }

    ReturnErrorOnFailure(GeneratePeerId(fabricId, nodeId, &nocPeerId));
    nocPubkey = P256PublicKey(certificates.GetLastCert()[0].mPublicKey);

    return CHIP_NO_ERROR;
}

FabricTable::~FabricTable()
{
    FabricTableDelegate * delegate = mDelegate;
    while (delegate)
    {
        FabricTableDelegate * temp = delegate->mNext;
        if (delegate->mOwnedByFabricTable)
        {
            chip::Platform::Delete(delegate);
        }
        delegate = temp;
    }
}

FabricInfo * FabricTable::FindFabric(P256PublicKeySpan rootPubKey, FabricId fabricId)
{
    for (auto & fabric : mStates)
    {
        if (!fabric.IsInitialized())
        {
            continue;
        }
        P256PublicKeySpan candidatePubKey;
        if (fabric.GetRootPubkey(candidatePubKey) != CHIP_NO_ERROR)
        {
            continue;
        }
        if (rootPubKey.data_equal(candidatePubKey) && fabricId == fabric.GetFabricId())
        {
            return &fabric;
        }
    }
    return nullptr;
}

FabricInfo * FabricTable::FindFabricWithIndex(FabricIndex fabricIndex)
{
    for (auto & fabric : mStates)
    {
        if (!fabric.IsInitialized())
        {
            continue;
        }

        if (fabric.GetFabricIndex() == fabricIndex)
        {
            return &fabric;
        }
    }

    return nullptr;
}

FabricInfo * FabricTable::FindFabricWithCompressedId(CompressedFabricId fabricId)
{
    for (auto & fabric : mStates)
    {
        if (!fabric.IsInitialized())
        {
            continue;
        }

        if (fabricId == fabric.GetPeerId().GetCompressedFabricId())
        {
            return &fabric;
        }
    }
    return nullptr;
}

CHIP_ERROR FabricTable::Store(FabricIndex index)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    FabricInfo * fabric = nullptr;

    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    fabric = FindFabricWithIndex(index);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = fabric->CommitToStorage(mStorage);
exit:
    if (err == CHIP_NO_ERROR && mDelegate != nullptr)
    {
        ChipLogProgress(Discovery, "Fabric (%d) persisted to storage. Calling OnFabricPersistedToStorage", index);
        FabricTableDelegate * delegate = mDelegate;
        while (delegate)
        {
            delegate->OnFabricPersistedToStorage(fabric);
            delegate = delegate->mNext;
        }
    }
    return err;
}

CHIP_ERROR FabricTable::LoadFromStorage(FabricInfo * fabric)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (!fabric->IsInitialized())
    {
        ReturnErrorOnFailure(fabric->LoadFromStorage(mStorage));

        FabricTableDelegate * delegate = mDelegate;
        while (delegate)
        {
            ChipLogProgress(Discovery, "Fabric (%d) loaded from storage", fabric->GetFabricIndex());
            delegate->OnFabricRetrievedFromStorage(fabric);
            delegate = delegate->mNext;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::SetFabricInfo(FabricInfo & newFabric)
{
    P256PublicKey pubkey;
    ValidationContext validContext;
    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    SetOperationalKeypair(newFabric.GetOperationalKey());
    SetRootCert(newFabric.mRootCert);

    ChipLogProgress(Discovery, "Verifying the received credentials");
    ReturnErrorOnFailure(
        VerifyCredentials(newFabric.mNOCCert, newFabric.mICACert, validContext, mOperationalId, mFabricId, pubkey));

    SetICACert(newFabric.mICACert);
    SetNOCCert(newFabric.mNOCCert);
    SetVendorId(newFabric.GetVendorId());
    SetFabricLabel(newFabric.GetFabricLabel());
    ChipLogProgress(Discovery, "Added new fabric at index: 0x%x, Initialized: %d", static_cast<unsigned>(GetFabricIndex()),
                    IsInitialized());
    ChipLogProgress(Discovery, "Assigned compressed fabric ID: 0x" ChipLogFormatX64 ", node ID: 0x" ChipLogFormatX64,
                    ChipLogValueX64(mOperationalId.GetCompressedFabricId()), ChipLogValueX64(mOperationalId.GetNodeId()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTable::AddNewFabric(FabricInfo & newFabric, FabricIndex * outputIndex)
{
    VerifyOrReturnError(outputIndex != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");

    // Check whether we already have a matching fabric.  An incoming fabric does
    // not have its fabric id set yet, so we have to extract it here to do the
    // comparison.
    FabricId fabricId;
    {
        ByteSpan noc;
        ReturnErrorOnFailure(newFabric.GetNOCCert(noc));
        NodeId unused;
        ReturnErrorOnFailure(ExtractNodeIdFabricIdFromOpCert(noc, &unused, &fabricId));
    }
    for (auto & existingFabric : *this)
    {
        if (existingFabric.GetFabricId() == fabricId)
        {
            P256PublicKeySpan existingRootKey, newRootKey;
            ReturnErrorOnFailure(existingFabric.GetRootPubkey(existingRootKey));
            ReturnErrorOnFailure(newFabric.GetRootPubkey(newRootKey));
            if (existingRootKey.data_equal(newRootKey))
            {
                return CHIP_ERROR_FABRIC_EXISTS;
            }
        }
    }

    if (!mNextAvailableFabricIndex.HasValue())
    {
        // No more indices available.  Bail out.
        return CHIP_ERROR_NO_MEMORY;
    }

    // Find an available slot.
    for (auto & fabric : mStates)
    {
        if (!fabric.IsInitialized())
        {
            FabricIndex newFabricIndex = mNextAvailableFabricIndex.Value();
            fabric.mFabricIndex        = newFabricIndex;
            CHIP_ERROR err             = fabric.SetFabricInfo(newFabric);
            if (err != CHIP_NO_ERROR)
            {
                fabric.Reset();
                return err;
            }

            err = Store(newFabricIndex);
            if (err != CHIP_NO_ERROR)
            {
                fabric.Reset();
                FabricInfo::DeleteFromStorage(mStorage, newFabricIndex);
                return err;
            }

            UpdateNextAvailableFabricIndex();
            err = StoreFabricIndexInfo();
            if (err != CHIP_NO_ERROR)
            {
                // Roll everything back.
                mNextAvailableFabricIndex.SetValue(newFabricIndex);
                fabric.Reset();
                FabricInfo::DeleteFromStorage(mStorage, newFabricIndex);
            }
            else
            {
                *outputIndex = newFabricIndex;
                mFabricCount++;
            }
            return err;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR FabricTable::Delete(FabricIndex index)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    FabricInfo * fabric      = FindFabricWithIndex(index);
    bool fabricIsInitialized = fabric != nullptr && fabric->IsInitialized();
    CompressedFabricId compressedFabricId =
        fabricIsInitialized ? fabric->GetPeerId().GetCompressedFabricId() : kUndefinedCompressedFabricId;
    CHIP_ERROR err = FabricInfo::DeleteFromStorage(mStorage, index); // Delete from storage regardless
    if (!fabricIsInitialized)
    {
        // Make sure to return the error our API promises, not whatever storage
        // chose to return.
        return CHIP_ERROR_NOT_FOUND;
    }
    ReturnErrorOnFailure(err);

    // Since fabricIsInitialized was true, fabric is not null.
    fabric->Reset();

    // If we ever start moving the FabricInfo entries around in the array on
    // delete, we should update DeleteAllFabrics to handle that.

    if (!mNextAvailableFabricIndex.HasValue())
    {
        // We must have been in a situation where CHIP_CONFIG_MAX_FABRICS is 254
        // and our fabric table was full, so there was no valid next index.  We
        // have a single available index now, though; use it as
        // mNextAvailableFabricIndex.
        mNextAvailableFabricIndex.SetValue(index);
    }
    // If StoreFabricIndexInfo fails here, that's probably OK.  When we try to
    // read things from storage later we will realize there is nothing for this
    // index.
    StoreFabricIndexInfo();

    if (mDelegate != nullptr)
    {
        if (mFabricCount == 0)
        {
            ChipLogError(Discovery, "Trying to delete a fabric, but the current fabric count is already 0");
        }
        else
        {
            mFabricCount--;
            ChipLogProgress(Discovery, "Fabric (0x%x) deleted. Calling OnFabricDeletedFromStorage", static_cast<unsigned>(index));
        }

        FabricTableDelegate * delegate = mDelegate;
        while (delegate)
        {
            delegate->OnFabricDeletedFromStorage(compressedFabricId, index);
            delegate = delegate->mNext;
        }
    }
    return CHIP_NO_ERROR;
}

void FabricTable::DeleteAllFabrics()
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (auto & fabric : *this)
    {
        Delete(fabric.GetFabricIndex());
    }
}

CHIP_ERROR FabricTable::Init(PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStorage = storage;
    ChipLogDetail(Discovery, "Init fabric pairing table with server storage");

    // Load the current fabrics from the storage. This is done here, since ConstFabricIterator
    // iterator doesn't have mechanism to load fabric info from storage on demand.
    // TODO - Update ConstFabricIterator to load fabric info from storage
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");

    mFabricCount = 0;
    for (auto & fabric : mStates)
    {
        fabric.Reset();
    }
    mNextAvailableFabricIndex.SetValue(kMinValidFabricIndex);

    uint8_t buf[IndexInfoTLVMaxSize()];
    uint16_t size = sizeof(buf);
    DefaultStorageKeyAllocator keyAlloc;
    CHIP_ERROR err = mStorage->SyncGetKeyValue(keyAlloc.FabricIndexInfo(), buf, size);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // No fabrics yet.  Nothing to be done here.
    }
    else
    {
        ReturnErrorOnFailure(err);
        TLV::ContiguousBufferTLVReader reader;
        reader.Init(buf, size);

        ReturnErrorOnFailure(ReadFabricInfo(reader));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTable::AddFabricDelegate(FabricTableDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    for (FabricTableDelegate * iter = mDelegate; iter != nullptr; iter = iter->mNext)
    {
        if (iter == delegate)
        {
            return CHIP_NO_ERROR;
        }
    }
    delegate->mNext = mDelegate;
    mDelegate       = delegate;
    ChipLogDetail(Discovery, "Add fabric pairing table delegate");
    return CHIP_NO_ERROR;
}

namespace {
// Increment a fabric index in a way that ensures that it stays in the valid
// range [kMinValidFabricIndex, kMaxValidFabricIndex].
FabricIndex NextFabricIndex(FabricIndex index)
{
    if (index == kMaxValidFabricIndex)
    {
        return kMinValidFabricIndex;
    }

    return static_cast<FabricIndex>(index + 1);
}
} // anonymous namespace

void FabricTable::UpdateNextAvailableFabricIndex()
{
    // Only called when mNextAvailableFabricIndex.HasValue()
    for (FabricIndex candidate = NextFabricIndex(mNextAvailableFabricIndex.Value()); candidate != mNextAvailableFabricIndex.Value();
         candidate             = NextFabricIndex(candidate))
    {
        if (!FindFabricWithIndex(candidate))
        {
            mNextAvailableFabricIndex.SetValue(candidate);
            return;
        }
    }

    mNextAvailableFabricIndex.ClearValue();
}

CHIP_ERROR FabricTable::StoreFabricIndexInfo() const
{
    uint8_t buf[IndexInfoTLVMaxSize()];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

    if (mNextAvailableFabricIndex.HasValue())
    {
        writer.Put(kNextAvailableFabricIndexTag, mNextAvailableFabricIndex.Value());
    }
    else
    {
        writer.PutNull(kNextAvailableFabricIndexTag);
    }

    TLV::TLVType innerContainerType;
    ReturnErrorOnFailure(writer.StartContainer(kFabricIndicesTag, TLV::kTLVType_Array, innerContainerType));
    // Only enumerate the fabrics that are initialized.
    for (const auto & fabric : *this)
    {
        writer.Put(TLV::AnonymousTag(), fabric.GetFabricIndex());
    }

    ReturnErrorOnFailure(writer.EndContainer(innerContainerType));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto indexInfoLength = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(indexInfoLength), CHIP_ERROR_BUFFER_TOO_SMALL);

    DefaultStorageKeyAllocator keyAlloc;
    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(keyAlloc.FabricIndexInfo(), buf, static_cast<uint16_t>(indexInfoLength)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTable::ReadFabricInfo(TLV::ContiguousBufferTLVReader & reader)
{
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ReturnErrorOnFailure(reader.Next(kNextAvailableFabricIndexTag));
    if (reader.GetType() == TLV::kTLVType_Null)
    {
        mNextAvailableFabricIndex.ClearValue();
    }
    else
    {
        ReturnErrorOnFailure(reader.Get(mNextAvailableFabricIndex.Emplace()));
    }

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, kFabricIndicesTag));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    CHIP_ERROR err;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (mFabricCount >= ArraySize(mStates))
        {
            // We have nowhere to deserialize this fabric info into.
            return CHIP_ERROR_NO_MEMORY;
        }

        auto & fabric = mStates[mFabricCount];
        ReturnErrorOnFailure(reader.Get(fabric.mFabricIndex));

        err = LoadFromStorage(&fabric);
        if (err == CHIP_NO_ERROR)
        {
            ++mFabricCount;
        }
        else
        {
            // This could happen if we failed to store our fabric index info
            // after we deleted the fabric from storage.  Just ignore this
            // fabric index and keep going.
        }
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    ReturnErrorOnFailure(reader.ExitContainer(containerType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    if (!mNextAvailableFabricIndex.HasValue() && mFabricCount < kMaxValidFabricIndex)
    {
        // We must have a fabric index available here. This situation could
        // happen if we fail to store fabric index info when deleting a
        // fabric.
        mNextAvailableFabricIndex.SetValue(kMinValidFabricIndex);
        if (FindFabricWithIndex(kMinValidFabricIndex))
        {
            UpdateNextAvailableFabricIndex();
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::TestOnlyBuildFabric(ByteSpan rootCert, ByteSpan icacCert, ByteSpan nocCert, ByteSpan nodePubKey,
                                           ByteSpan nodePrivateKey)
{
    Reset();

    ReturnErrorOnFailure(SetRootCert(rootCert));
    ReturnErrorOnFailure(SetICACert(icacCert));
    ReturnErrorOnFailure(SetNOCCert(nocCert));

    // NOTE: this requres ENABLE_HSM_CASE_OPS_KEY is not defined
    P256SerializedKeypair opKeysSerialized;
    memcpy(static_cast<uint8_t *>(opKeysSerialized), nodePubKey.data(), nodePubKey.size());
    memcpy(static_cast<uint8_t *>(opKeysSerialized) + nodePubKey.size(), nodePrivateKey.data(), nodePrivateKey.size());
    ReturnErrorOnFailure(opKeysSerialized.SetLength(nodePubKey.size() + nodePrivateKey.size()));

    P256Keypair opKey;
    ReturnErrorOnFailure(opKey.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(SetOperationalKeypair(&opKey));

    // NOTE: mVendorId and mFabricLabel are not initialize, because they are not used in tests.
    return CHIP_NO_ERROR;
}

} // namespace chip
