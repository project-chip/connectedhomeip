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

static_assert(kMinValidFabricIndex <= CHIP_CONFIG_MAX_FABRICS, "Must support some fabrics.");
static_assert(CHIP_CONFIG_MAX_FABRICS <= kMaxValidFabricIndex, "Max fabric count out of range.");

// Tags for our metadata storage.
constexpr TLV::Tag kVendorIdTag    = TLV::ContextTag(0);
constexpr TLV::Tag kFabricLabelTag = TLV::ContextTag(1);

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
        uint8_t buf[OpKeyTLVMaxSize()];
        TLV::TLVWriter writer;
        writer.Init(buf);

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

        ReturnErrorOnFailure(writer.Put(kOpKeyVersionTag, kOpKeyVersion));

        // If key storage is externally managed, key is not stored here,
        // and when loading is done later, it will be ignored.
        if (!mHasExternallyOwnedOperationalKey && (mOperationalKey != nullptr))
        {
            Crypto::P256SerializedKeypair serializedOpKey;
            ReturnErrorOnFailure(mOperationalKey->Serialize(serializedOpKey));
            ReturnErrorOnFailure(writer.Put(kOpKeyDataTag, ByteSpan(serializedOpKey.Bytes(), serializedOpKey.Length())));
        }

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

    ChipLogProgress(FabricProvisioning, "Loading from storage for fabric index 0x%x", static_cast<unsigned>(mFabricIndex));

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
        ReturnErrorOnFailure(GeneratePeerId(mRootCert, mFabricId, nodeId, &mOperationalId));
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

        CHIP_ERROR err = reader.Next(kOpKeyDataTag);
        if (err == CHIP_NO_ERROR)
        {
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
        }
        else
        {
            // Key was absent: set mOperationalKey to null, for another caller to set
            // it. This may happen if externally owned.
            mOperationalKey = nullptr;
        }

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

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        ReturnErrorOnFailure(reader.VerifyEndOfContainer());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::GeneratePeerId(const ByteSpan & rcac, FabricId fabricId, NodeId nodeId, PeerId * compressedPeerId)
{
    ReturnErrorCodeIf(compressedPeerId == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    uint8_t compressedFabricIdBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricIdSpan(compressedFabricIdBuf);
    P256PublicKey rootPubkey;

    {
        P256PublicKeySpan rootPubkeySpan;
        ReturnErrorOnFailure(ExtractPublicKeyFromChipCert(rcac, rootPubkeySpan));
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
        ChipLogDetail(FabricProvisioning, "Error deleting part of fabric %d: %" CHIP_ERROR_FORMAT, fabricIndex,
                      prevDeleteErr.Format());
    }
    return prevDeleteErr;
}

CHIP_ERROR FabricInfo::SetOperationalKeypair(const P256Keypair * keyPair)
{
    VerifyOrReturnError(keyPair != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    P256SerializedKeypair serialized;
    ReturnErrorOnFailure(keyPair->Serialize(serialized));

    if (mHasExternallyOwnedOperationalKey)
    {
        // Drop it, so we will allocate an internally owned one.
        mOperationalKey                   = nullptr;
        mHasExternallyOwnedOperationalKey = false;
    }

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

CHIP_ERROR FabricInfo::SetExternallyOwnedOperationalKeypair(P256Keypair * keyPair)
{
    VerifyOrReturnError(keyPair != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (!mHasExternallyOwnedOperationalKey && mOperationalKey != nullptr)
    {
        chip::Platform::Delete(mOperationalKey);
        mOperationalKey = nullptr;
    }

    mHasExternallyOwnedOperationalKey = true;
    mOperationalKey                   = keyPair;
    return CHIP_NO_ERROR;
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
    return VerifyCredentials(noc, icac, mRootCert, context, nocPeerId, fabricId, nocPubkey);
}

CHIP_ERROR FabricInfo::VerifyCredentials(const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                         ValidationContext & context, PeerId & nocPeerId, FabricId & fabricId,
                                         Crypto::P256PublicKey & nocPubkey)
{
    // TODO - Optimize credentials verification logic
    //        The certificate chain construction and verification is a compute and memory intensive operation.
    //        It can be optimized by not loading certificate (i.e. rcac) that's local and implicitly trusted.
    //        The FindValidCert() algorithm will need updates to achieve this refactor.
    constexpr uint8_t kMaxNumCertsInOpCreds = 3;

    ChipCertificateSet certificates;
    ReturnErrorOnFailure(certificates.Init(kMaxNumCertsInOpCreds));

    ReturnErrorOnFailure(certificates.LoadCert(rcac, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

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

    ReturnErrorOnFailure(GeneratePeerId(rcac, fabricId, nodeId, &nocPeerId));
    nocPubkey = P256PublicKey(certificates.GetLastCert()[0].mPublicKey);

    return CHIP_NO_ERROR;
}

FabricTable::~FabricTable()
{
    // Remove all links to every delegate
    FabricTable::Delegate * delegate = mDelegateListRoot;
    while (delegate)
    {
        FabricTable::Delegate * temp = delegate->next;
        delegate->next               = nullptr;
        delegate                     = temp;
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

CHIP_ERROR FabricTable::Store(FabricIndex fabricIndex)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    FabricInfo * fabric = nullptr;

    VerifyOrExit(mStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    fabric = FindFabricWithIndex(fabricIndex);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = fabric->CommitToStorage(mStorage);
exit:
    if (err == CHIP_NO_ERROR && mDelegateListRoot != nullptr)
    {
        ChipLogProgress(FabricProvisioning, "Fabric (0x%x) persisted to storage. Calling OnFabricPersistedToStorage",
                        static_cast<unsigned>(fabricIndex));
        FabricTable::Delegate * delegate = mDelegateListRoot;
        while (delegate)
        {
            delegate->OnFabricPersistedToStorage(*this, fabricIndex);
            delegate = delegate->next;
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

        FabricTable::Delegate * delegate = mDelegateListRoot;
        while (delegate)
        {
            ChipLogProgress(FabricProvisioning, "Fabric (0x%x) loaded from storage",
                            static_cast<unsigned>(fabric->GetFabricIndex()));
            delegate->OnFabricRetrievedFromStorage(*this, fabric->GetFabricIndex());
            delegate = delegate->next;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricInfo::SetFabricInfo(FabricInfo & newFabric, Credentials::CertificateValidityPolicy * policy)
{
    P256PublicKey pubkey;
    ValidationContext validContext;
    // Note that we do NOT set a time in the validation context.  This will
    // cause the certificate chain NotBefore / NotAfter time validation logic
    // to report CertificateValidityResult::kTimeUnknown.
    //
    // The default CHIPCert policy passes NotBefore / NotAfter validation for
    // this case where time is unknown.  If an override policy is passed, it
    // will be up to the passed policy to decide how to handle this.
    //
    // In the FabricTable::AddNewFabric and FabricTable::UpdateFabric calls,
    // the passed policy always passes for all questions of time validity.  The
    // rationale is that installed certificates should be valid at the time of
    // installation by definition.  If they are not and the commissionee and
    // commissioner disagree enough on current time, CASE will fail and our
    // fail-safe timer will expire.
    //
    // This then is ultimately how we validate that NotBefore / NotAfter in
    // newly installed certificates is workable.
    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    validContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    validContext.mValidityPolicy = policy;

    // Make sure to not modify any of our state until VerifyCredentials passes.
    PeerId operationalId;
    FabricId fabricId;
    ChipLogProgress(FabricProvisioning, "Verifying the received credentials");
    CHIP_ERROR err = VerifyCredentials(newFabric.mNOCCert, newFabric.mICACert, newFabric.mRootCert, validContext, operationalId,
                                       fabricId, pubkey);
    if (err != CHIP_NO_ERROR && err != CHIP_ERROR_WRONG_NODE_ID)
    {
        err = CHIP_ERROR_UNSUPPORTED_CERT_FORMAT;
    }
    ReturnErrorOnFailure(err);

    auto * operationalKey = newFabric.GetOperationalKey();
    if (operationalKey == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Verify that public key in NOC matches public key generated by node and sent in CSRResponse message.
    VerifyOrReturnError(operationalKey->Pubkey().Length() == pubkey.Length(), CHIP_ERROR_INVALID_PUBLIC_KEY);
    VerifyOrReturnError(memcmp(operationalKey->Pubkey().ConstBytes(), pubkey.Bytes(), pubkey.Length()) == 0,
                        CHIP_ERROR_INVALID_PUBLIC_KEY);

    if (mFabricId != kUndefinedFabricId)
    {
        VerifyOrReturnError(mFabricId == fabricId, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);
    }

    if (newFabric.mHasExternallyOwnedOperationalKey)
    {
        ReturnErrorOnFailure(SetExternallyOwnedOperationalKeypair(operationalKey));
    }
    else
    {
        ReturnErrorOnFailure(SetOperationalKeypair(operationalKey));
    }

    SetRootCert(newFabric.mRootCert);
    mOperationalId = operationalId;
    mFabricId      = fabricId;
    SetICACert(newFabric.mICACert);
    SetNOCCert(newFabric.mNOCCert);
    SetVendorId(newFabric.GetVendorId());
    SetFabricLabel(newFabric.GetFabricLabel());
    ChipLogProgress(FabricProvisioning, "Added new fabric at index: 0x%x, Initialized: %d", static_cast<unsigned>(GetFabricIndex()),
                    IsInitialized());
    ChipLogProgress(FabricProvisioning, "Assigned compressed fabric ID: 0x" ChipLogFormatX64 ", node ID: 0x" ChipLogFormatX64,
                    ChipLogValueX64(mOperationalId.GetCompressedFabricId()), ChipLogValueX64(mOperationalId.GetNodeId()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricTable::AddNewFabricForTest(FabricInfo & newFabric, FabricIndex * outputIndex)
{
    VerifyOrReturnError(outputIndex != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return AddNewFabricInner(newFabric, outputIndex);
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

    return AddNewFabricInner(newFabric, outputIndex);
}

/*
 * A validation policy we can pass into VerifyCredentials to extract the
 * latest NotBefore time in the certificate chain without having to load the
 * certificates into memory again, and one which will pass validation for all
 * questions of NotBefore / NotAfter validity.
 *
 * The rationale is that installed certificates should be valid at the time of
 * installation by definition.  If they are not and the commissionee and
 * commissioner disagree enough on current time, CASE will fail and our
 * fail-safe timer will expire.
 *
 * This then is ultimately how we validate that NotBefore / NotAfter in
 * newly installed certificates is workable.
 */
class NotBeforeCollector : public Credentials::CertificateValidityPolicy
{
public:
    NotBeforeCollector() : mLatestNotBefore(0) {}
    CHIP_ERROR ApplyCertificateValidityPolicy(const ChipCertificateData * cert, uint8_t depth,
                                              CertificateValidityResult result) override
    {
        if (cert->mNotBeforeTime > mLatestNotBefore.count())
        {
            mLatestNotBefore = System::Clock::Seconds32(cert->mNotBeforeTime);
        }
        return CHIP_NO_ERROR;
    }
    System::Clock::Seconds32 mLatestNotBefore;
};

CHIP_ERROR FabricTable::UpdateFabric(FabricIndex fabricIndex, FabricInfo & newFabricInfo)
{
    FabricInfo * fabricInfo = FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    NotBeforeCollector notBeforeCollector;
    ReturnErrorOnFailure(fabricInfo->SetFabricInfo(newFabricInfo, &notBeforeCollector));
    ReturnErrorOnFailure(Store(fabricIndex));
    // Update failure of Last Known Good Time is non-fatal.  If Last
    // Known Good Time is unknown during incoming certificate validation
    // for CASE and current time is also unknown, the certificate
    // validity policy will see this condition and can act appropriately.
    mLastKnownGoodTime.UpdateLastKnownGoodChipEpochTime(notBeforeCollector.mLatestNotBefore);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
FabricTable::AddNewFabricInner(FabricInfo & newFabric, FabricIndex * outputIndex)
{
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
            NotBeforeCollector notBeforeCollector;
            FabricIndex newFabricIndex = mNextAvailableFabricIndex.Value();
            fabric.mFabricIndex        = newFabricIndex;
            CHIP_ERROR err;
            if ((err = fabric.SetFabricInfo(newFabric, &notBeforeCollector)) != CHIP_NO_ERROR)
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
            // Update failure of Last Known Good Time is non-fatal.  If Last
            // Known Good Time is unknown during incoming certificate validation
            // for CASE and current time is also unknown, the certificate
            // validity policy will see this condition and can act appropriately.
            mLastKnownGoodTime.UpdateLastKnownGoodChipEpochTime(notBeforeCollector.mLatestNotBefore);
            if ((err = StoreFabricIndexInfo()) != CHIP_NO_ERROR)
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

CHIP_ERROR FabricTable::Delete(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    FabricInfo * fabric      = FindFabricWithIndex(fabricIndex);
    bool fabricIsInitialized = fabric != nullptr && fabric->IsInitialized();
    CHIP_ERROR err           = FabricInfo::DeleteFromStorage(mStorage, fabricIndex); // Delete from storage regardless
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
        mNextAvailableFabricIndex.SetValue(fabricIndex);
    }
    // If StoreFabricIndexInfo fails here, that's probably OK.  When we try to
    // read things from storage later we will realize there is nothing for this
    // index.
    StoreFabricIndexInfo();

    if (mDelegateListRoot != nullptr)
    {
        if (mFabricCount == 0)
        {
            ChipLogError(FabricProvisioning, "Trying to delete a fabric, but the current fabric count is already 0");
        }
        else
        {
            mFabricCount--;
            ChipLogProgress(FabricProvisioning, "Fabric (0x%x) deleted. Calling OnFabricDeletedFromStorage",
                            static_cast<unsigned>(fabricIndex));
        }

        FabricTable::Delegate * delegate = mDelegateListRoot;
        while (delegate)
        {
            delegate->OnFabricDeletedFromStorage(*this, fabricIndex);
            delegate = delegate->next;
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
    ChipLogDetail(FabricProvisioning, "Init fabric pairing table with server storage");

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

    // Init failure of Last Known Good Time is non-fatal.  If Last Known Good
    // Time is unknown during incoming certificate validation for CASE and
    // current time is also unknown, the certificate validity policy will see
    // this condition and can act appropriately.
    mLastKnownGoodTime.Init(storage);

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

CHIP_ERROR FabricTable::AddFabricDelegate(FabricTable::Delegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    for (FabricTable::Delegate * iter = mDelegateListRoot; iter != nullptr; iter = iter->next)
    {
        if (iter == delegate)
        {
            return CHIP_NO_ERROR;
        }
    }
    delegate->next    = mDelegateListRoot;
    mDelegateListRoot = delegate;
    return CHIP_NO_ERROR;
}

void FabricTable::RemoveFabricDelegate(FabricTable::Delegate * delegateToRemove)
{
    VerifyOrReturn(delegateToRemove != nullptr);

    if (delegateToRemove == mDelegateListRoot)
    {
        // Removing head of the list, keep link to next, may
        // be nullptr.
        mDelegateListRoot = mDelegateListRoot->next;
    }
    else
    {
        // Removing some other item: check if next, and
        // remove the link, keeping its neighbour.
        FabricTable::Delegate * currentNode = mDelegateListRoot;

        while (currentNode)
        {
            if (currentNode->next == delegateToRemove)
            {
                FabricTable::Delegate * temp = delegateToRemove->next;
                currentNode->next            = temp;
                delegateToRemove->next       = nullptr;
                return;
            }

            currentNode = currentNode->next;
        }
    }
}

CHIP_ERROR FabricTable::SetLastKnownGoodChipEpochTime(System::Clock::Seconds32 lastKnownGoodChipEpochTime)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Find our latest NotBefore time for any installed certificate.
    System::Clock::Seconds32 latestNotBefore = System::Clock::Seconds32(0);
    for (auto & fabric : mStates)
    {
        if (!fabric.IsInitialized())
        {
            continue;
        }
        {
            ByteSpan rcac;
            SuccessOrExit(err = fabric.GetRootCert(rcac));
            chip::System::Clock::Seconds32 rcacNotBefore;
            SuccessOrExit(err = Credentials::ExtractNotBeforeFromChipCert(rcac, rcacNotBefore));
            latestNotBefore = rcacNotBefore > latestNotBefore ? rcacNotBefore : latestNotBefore;
        }
        {
            ByteSpan icac;
            SuccessOrExit(err = fabric.GetICACert(icac));
            if (!icac.empty())
            {
                chip::System::Clock::Seconds32 icacNotBefore;
                ReturnErrorOnFailure(Credentials::ExtractNotBeforeFromChipCert(icac, icacNotBefore));
                latestNotBefore = icacNotBefore > latestNotBefore ? icacNotBefore : latestNotBefore;
            }
        }
        {
            ByteSpan noc;
            SuccessOrExit(err = fabric.GetNOCCert(noc));
            chip::System::Clock::Seconds32 nocNotBefore;
            ReturnErrorOnFailure(Credentials::ExtractNotBeforeFromChipCert(noc, nocNotBefore));
            latestNotBefore = nocNotBefore > latestNotBefore ? nocNotBefore : latestNotBefore;
        }
    }
    // Pass this to the LastKnownGoodTime object so it can make determination
    // of the legality of our new proposed time.
    SuccessOrExit(err = mLastKnownGoodTime.SetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime, latestNotBefore));
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(FabricProvisioning, "Failed to update Known Good Time: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

namespace {
// Increment a fabric index in a way that ensures that it stays in the valid
// range [kMinValidFabricIndex, kMaxValidFabricIndex].
FabricIndex NextFabricIndex(FabricIndex fabricIndex)
{
    if (fabricIndex == kMaxValidFabricIndex)
    {
        return kMinValidFabricIndex;
    }

    return static_cast<FabricIndex>(fabricIndex + 1);
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

CHIP_ERROR FabricInfo::TestOnlyBuildFabric(ByteSpan rootCert, ByteSpan icacCert, ByteSpan nocCert, ByteSpan nocKey)
{
    Reset();

    ReturnErrorOnFailure(SetRootCert(rootCert));
    ReturnErrorOnFailure(SetICACert(icacCert));
    ReturnErrorOnFailure(SetNOCCert(nocCert));

    // NOTE: this requres ENABLE_HSM_CASE_OPS_KEY is not defined
    P256SerializedKeypair opKeysSerialized;
    memcpy(static_cast<uint8_t *>(opKeysSerialized), nocKey.data(), nocKey.size());
    ReturnErrorOnFailure(opKeysSerialized.SetLength(nocKey.size()));

    P256Keypair opKey;
    ReturnErrorOnFailure(opKey.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(SetOperationalKeypair(&opKey));

    // NOTE: mVendorId and mFabricLabel are not initialize, because they are not used in tests.
    return CHIP_NO_ERROR;
}

} // namespace chip
