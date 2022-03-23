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

// Tags for our operational keypair storage.
constexpr TLV::Tag kOpKeyVersionTag = TLV::ContextTag(0);
constexpr TLV::Tag kOpKeyDataTag    = TLV::ContextTag(1);

// If this version grows beyond UINT16_MAX, adjust OpKeypairTLVMaxSize
// accordingly.
constexpr uint16_t kOpKeyVersion = 1;
} // anonymous namespace

CHIP_ERROR FabricInfo::CommitToStorage(PersistentStorageDelegate * storage)
{
    DefaultStorageKeyAllocator keyAlloc;

    VerifyOrReturnError(mRootCert.size() <= kMaxCHIPCertLength && mICACert.size() <= kMaxCHIPCertLength &&
                            mNOCCert.size() <= kMaxCHIPCertLength,
                        CHIP_ERROR_BUFFER_TOO_SMALL);
    static_assert(kMaxCHIPCertLength <= UINT16_MAX, "Casting to uint16_t won't be safe");

    ReturnErrorOnFailure(
        storage->SyncSetKeyValue(keyAlloc.FabricRCAC(mFabric), mRootCert.data(), static_cast<uint16_t>(mRootCert.size())));

    // Workaround for the fact that some storage backends do not allow storing
    // a nullptr with 0 length.  See
    // https://github.com/project-chip/connectedhomeip/issues/16030.
    if (!mICACert.empty())
    {
        ReturnErrorOnFailure(
            storage->SyncSetKeyValue(keyAlloc.FabricICAC(mFabric), mICACert.data(), static_cast<uint16_t>(mICACert.size())));
    }
    else
    {
        // Make sure there is no stale data.
        CHIP_ERROR err = storage->SyncDeleteKeyValue(keyAlloc.FabricICAC(mFabric));
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ReturnErrorOnFailure(err);
        }
    }

    ReturnErrorOnFailure(
        storage->SyncSetKeyValue(keyAlloc.FabricNOC(mFabric), mNOCCert.data(), static_cast<uint16_t>(mNOCCert.size())));

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
        ReturnErrorOnFailure(storage->SyncSetKeyValue(keyAlloc.FabricOpKey(mFabric), buf, static_cast<uint16_t>(opKeyLength)));
    }

    {
        uint8_t buf[MetadataTLVMaxSize()];
        TLV::TLVWriter writer;
        writer.Init(buf);

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

        ReturnErrorOnFailure(writer.Put(kVendorIdTag, mVendorId));

        ReturnErrorOnFailure(writer.PutString(kFabricLabelTag, CharSpan::fromCharString(mFabricLabel)));

        ReturnErrorOnFailure(writer.EndContainer(outerType));

        const auto metadataLength = writer.GetLengthWritten();
        VerifyOrReturnError(CanCastTo<uint16_t>(metadataLength), CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(
            storage->SyncSetKeyValue(keyAlloc.FabricMetadata(mFabric), buf, static_cast<uint16_t>(metadataLength)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::LoadFromStorage(PersistentStorageDelegate * storage)
{
    DefaultStorageKeyAllocator keyAlloc;

    ChipLogProgress(Inet, "Loading from storage for fabric index %u", mFabric);

    // Scopes for "size" so we don't forget to re-initialize it between gets,
    // since each get modifies it.
    {
        uint8_t buf[Credentials::kMaxCHIPCertLength];
        uint16_t size = sizeof(buf);
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricRCAC(mFabric), buf, size));
        ReturnErrorOnFailure(SetRootCert(ByteSpan(buf, size)));
    }

    {
        uint8_t buf[Credentials::kMaxCHIPCertLength];
        uint16_t size  = sizeof(buf);
        CHIP_ERROR err = storage->SyncGetKeyValue(keyAlloc.FabricICAC(mFabric), buf, size);
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
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricNOC(mFabric), buf, size));
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
        uint8_t buf[OpKeyTLVMaxSize()];
        uint16_t size = sizeof(buf);
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricOpKey(mFabric), buf, size));
        TLV::ContiguousBufferTLVReader reader;
        reader.Init(buf, size);

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
            mOperationalKey->SetKeyId(CASE_OPS_KEY);
#else
            mOperationalKey = chip::Platform::New<P256Keypair>();
#endif
        }
        VerifyOrReturnError(mOperationalKey != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(mOperationalKey->Deserialize(serializedOpKey));
#ifdef ENABLE_HSM_CASE_OPS_KEY
        // Set provisioned_key = true , so that key is not deleted from HSM.
        mOperationalKey->provisioned_key = true;
#endif

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        ReturnErrorOnFailure(reader.VerifyEndOfContainer());
    }

    {
        uint8_t buf[MetadataTLVMaxSize()];
        uint16_t size = sizeof(buf);
        ReturnErrorOnFailure(storage->SyncGetKeyValue(keyAlloc.FabricMetadata(mFabric), buf, size));
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

    ChipLogDetail(Inet, "Generating compressed fabric ID using uncompressed fabric ID 0x" ChipLogFormatX64 " and root pubkey",
                  ChipLogValueX64(fabricId));
    ChipLogByteSpan(Inet, ByteSpan(rootPubkey.ConstBytes(), rootPubkey.Length()));
    ReturnErrorOnFailure(GenerateCompressedFabricId(rootPubkey, fabricId, compressedFabricIdSpan));
    ChipLogDetail(Inet, "Generated compressed fabric ID");
    ChipLogByteSpan(Inet, compressedFabricIdSpan);

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
        mOperationalKey->SetKeyId(CASE_OPS_KEY);
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

CHIP_ERROR FabricInfo::GenerateDestinationID(const ByteSpan & ipk, const ByteSpan & random, NodeId destNodeId,
                                             MutableByteSpan & destinationId) const
{
    constexpr uint16_t kSigmaParamRandomNumberSize = 32;
    constexpr size_t kDestinationMessageLen =
        kSigmaParamRandomNumberSize + kP256_PublicKey_Length + sizeof(FabricId) + sizeof(NodeId);
    HMAC_sha hmac;
    uint8_t destinationMessage[kDestinationMessageLen];
    P256PublicKeySpan rootPubkeySpan;

    ReturnErrorOnFailure(GetRootPubkey(rootPubkeySpan));

    Encoding::LittleEndian::BufferWriter bbuf(destinationMessage, sizeof(destinationMessage));

    ChipLogDetail(Inet,
                  "Generating DestinationID. Fabric ID 0x" ChipLogFormatX64 ", Dest node ID 0x" ChipLogFormatX64 ", Random data",
                  ChipLogValueX64(mFabricId), ChipLogValueX64(destNodeId));
    ChipLogByteSpan(Inet, random);

    bbuf.Put(random.data(), random.size());
    // TODO: In the current implementation this check is required because in some cases the
    //       GenerateDestinationID() is called before mRootCert is initialized and GetRootPubkey() returns
    //       empty Span.
    if (!rootPubkeySpan.empty())
    {
        ChipLogDetail(Inet, "Root pubkey");
        ChipLogByteSpan(Inet, rootPubkeySpan);
        bbuf.Put(rootPubkeySpan.data(), rootPubkeySpan.size());
    }
    bbuf.Put64(mFabricId);
    bbuf.Put64(destNodeId);

    size_t written = 0;
    VerifyOrReturnError(bbuf.Fit(written), CHIP_ERROR_BUFFER_TOO_SMALL);

    ChipLogDetail(Inet, "IPK");
    ChipLogByteSpan(Inet, ipk);

    CHIP_ERROR err =
        hmac.HMAC_SHA256(ipk.data(), ipk.size(), destinationMessage, written, destinationId.data(), destinationId.size());
    ChipLogDetail(Inet, "Generated DestinationID output");
    ChipLogByteSpan(Inet, destinationId);
    return err;
}

CHIP_ERROR FabricInfo::MatchDestinationID(const ByteSpan & targetDestinationId, const ByteSpan & initiatorRandom,
                                          const ByteSpan * ipkList, size_t ipkListEntries)
{
    uint8_t localDestID[kSHA256_Hash_Length] = { 0 };
    MutableByteSpan localDestIDSpan(localDestID);
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    for (size_t ipkIdx = 0; ipkIdx < ipkListEntries; ++ipkIdx)
    {
        if (GenerateDestinationID(ipkList[ipkIdx], initiatorRandom, mOperationalId.GetNodeId(), localDestIDSpan) == CHIP_NO_ERROR &&
            targetDestinationId.data_equal(localDestIDSpan))
        {
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_CERT_NOT_TRUSTED;
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

void FabricTable::ReleaseFabricIndex(FabricIndex fabricIndex)
{
    FabricInfo * fabric = FindFabricWithIndex(fabricIndex);
    if (fabric != nullptr)
    {
        fabric->Reset();
    }
}

FabricInfo * FabricTable::FindFabric(P256PublicKeySpan rootPubKey, FabricId fabricId)
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric == nullptr)
        {
            continue;
        }
        P256PublicKeySpan candidatePubKey;
        if (fabric->GetRootPubkey(candidatePubKey) != CHIP_NO_ERROR)
        {
            continue;
        }
        if (rootPubKey.data_equal(candidatePubKey) && fabricId == fabric->GetFabricId())
        {
            LoadFromStorage(fabric);
            return fabric;
        }
    }
    return nullptr;
}

FabricInfo * FabricTable::FindFabricWithIndex(FabricIndex fabricIndex)
{
    if (fabricIndex >= kMinValidFabricIndex && fabricIndex <= kMaxValidFabricIndex)
    {
        FabricInfo * fabric = &mStates[fabricIndex - kMinValidFabricIndex];
        LoadFromStorage(fabric);
        return fabric;
    }

    return nullptr;
}

FabricInfo * FabricTable::FindFabricWithCompressedId(CompressedFabricId fabricId)
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);

        if (fabric != nullptr && fabricId == fabric->GetPeerId().GetCompressedFabricId())
        {
            LoadFromStorage(fabric);
            return fabric;
        }
    }
    return nullptr;
}

void FabricTable::Reset()
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);

        if (fabric != nullptr)
        {
            fabric->Reset();

            fabric->mFabric = i;
        }
    }
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
    }

    FabricTableDelegate * delegate = mDelegate;
    while (delegate)
    {
        ChipLogProgress(Discovery, "Fabric (%d) loaded from storage. Calling OnFabricRetrievedFromStorage",
                        fabric->GetFabricIndex());
        delegate->OnFabricRetrievedFromStorage(fabric);
        delegate = delegate->mNext;
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
    ChipLogProgress(Discovery, "Added new fabric at index: %d, Initialized: %d", GetFabricIndex(), IsInitialized());
    ChipLogProgress(Discovery, "Assigned compressed fabric ID: 0x" ChipLogFormatX64 ", node ID: 0x" ChipLogFormatX64,
                    ChipLogValueX64(mOperationalId.GetCompressedFabricId()), ChipLogValueX64(mOperationalId.GetNodeId()));
    return CHIP_NO_ERROR;
}

FabricIndex FabricTable::FindDestinationIDCandidate(const ByteSpan & destinationId, const ByteSpan & initiatorRandom,
                                                    const ByteSpan * ipkList, size_t ipkListEntries)
{
    static_assert(kMaxValidFabricIndex <= UINT8_MAX, "Cannot create more fabrics than UINT8_MAX");
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric != nullptr &&
            fabric->MatchDestinationID(destinationId, initiatorRandom, ipkList, ipkListEntries) == CHIP_NO_ERROR)
        {
            return i;
        }
    }

    return kUndefinedFabricIndex;
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

    for (FabricIndex i = mNextAvailableFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric != nullptr && !fabric->IsInitialized())
        {
            ReturnErrorOnFailure(fabric->SetFabricInfo(newFabric));
            ReturnErrorOnFailure(Store(i));
            mNextAvailableFabricIndex = static_cast<FabricIndex>((i + 1) % UINT8_MAX);
            *outputIndex              = i;
            mFabricCount++;
            return CHIP_NO_ERROR;
        }
    }

    for (FabricIndex i = kMinValidFabricIndex; i < kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = FindFabricWithIndex(i);
        if (fabric != nullptr && !fabric->IsInitialized())
        {
            ReturnErrorOnFailure(fabric->SetFabricInfo(newFabric));
            ReturnErrorOnFailure(Store(i));
            mNextAvailableFabricIndex = static_cast<FabricIndex>((i + 1) % UINT8_MAX);
            *outputIndex              = i;
            mFabricCount++;
            return CHIP_NO_ERROR;
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

    ReleaseFabricIndex(index);
    if (mDelegate != nullptr)
    {
        if (mFabricCount == 0)
        {
            ChipLogError(Discovery, "!!Trying to delete a fabric, but the current fabric count is already 0");
        }
        else
        {
            mFabricCount--;
        }
        ChipLogProgress(Discovery, "Fabric (%d) deleted. Calling OnFabricDeletedFromStorage", index);

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
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        Delete(i);
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
    for (FabricIndex i = kMinValidFabricIndex; i <= kMaxValidFabricIndex; i++)
    {
        FabricInfo * fabric = &mStates[i - kMinValidFabricIndex];
        if (LoadFromStorage(fabric) == CHIP_NO_ERROR)
        {
            mFabricCount++;
        }
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

CHIP_ERROR formatKey(FabricIndex fabricIndex, MutableCharSpan formattedKey, const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    int res = snprintf(formattedKey.data(), formattedKey.size(), "F%02X/%s", fabricIndex, key);
    if (res < 0 || (size_t) res >= formattedKey.size())
    {
        ChipLogError(Discovery, "Failed to format Key %s. snprintf error: %d", key, res);
        return CHIP_ERROR_NO_MEMORY;
    }
    return err;
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
