/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <algorithm>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>

namespace chip {
namespace Controller {

constexpr const char kOperationalCredentialsIssuerKeypairStorage[]             = "ExampleOpCredsCAKey";
constexpr const char kOperationalCredentialsIntermediateIssuerKeypairStorage[] = "ExampleOpCredsICAKey";
constexpr const char kOperationalCredentialsRootCertificateStorage[]           = "ExampleCARootCert";
constexpr const char kOperationalCredentialsIntermediateCertificateStorage[]   = "ExampleCAIntermediateCert";

using namespace Credentials;
using namespace Crypto;
using namespace TLV;

CHIP_ERROR ExampleOperationalCredentialsIssuer::Initialize(PersistentStorageDelegate & storage)
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;
    CHIP_ERROR err;

    // Initializing the default start validity to start of 2021. The default validity duration is 10 years.
    CHIP_ZERO_AT(effectiveTime);
    effectiveTime.Year  = 2021;
    effectiveTime.Month = 1;
    effectiveTime.Day   = 1;
    ReturnErrorOnFailure(ASN1ToChipEpochTime(effectiveTime, mNow));

    Crypto::P256SerializedKeypair serializedKey;
    uint16_t keySize = static_cast<uint16_t>(serializedKey.Capacity());

    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIssuerKeypairStorage, key,
                      err = storage.SyncGetKeyValue(key, serializedKey.Bytes(), keySize));
    serializedKey.SetLength(keySize);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s", kOperationalCredentialsIssuerKeypairStorage, ErrorStr(err));
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIssuer.Initialize());
        ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));

        keySize = static_cast<uint16_t>(serializedKey.Capacity());

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIssuerKeypairStorage, key,
                          ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(), keySize)));
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIssuer.Deserialize(serializedKey));
    }

    keySize = static_cast<uint16_t>(serializedKey.Capacity());

    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateIssuerKeypairStorage, key,
                      err = storage.SyncGetKeyValue(key, serializedKey.Bytes(), keySize));
    serializedKey.SetLength(keySize);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Couldn't get %s from storage: %s", kOperationalCredentialsIntermediateIssuerKeypairStorage,
                        ErrorStr(err));
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIntermediateIssuer.Initialize());
        ReturnErrorOnFailure(mIntermediateIssuer.Serialize(serializedKey));

        keySize = static_cast<uint16_t>(serializedKey.Capacity());

        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateIssuerKeypairStorage, key,
                          ReturnErrorOnFailure(storage.SyncSetKeyValue(key, serializedKey.Bytes(), keySize)));
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIntermediateIssuer.Deserialize(serializedKey));
    }

    mStorage     = &storage;
    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId,
                                                                                const Crypto::P256PublicKey & pubkey,
                                                                                MutableByteSpan & rcac, MutableByteSpan & icac,
                                                                                MutableByteSpan & noc)
{
    ChipDN noc_dn;
    // TODO: Is there a way to make this code less error-prone for consumers?
    // The consumer doesn't need to know the exact OID value.
    noc_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipFabricId, fabricId);
    noc_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipNodeId, nodeId);
    // TODO: Add support for the CASE Authenticated Tags attributes
    ChipDN icac_dn;
    icac_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipICAId, mIntermediateIssuerId);
    ChipDN rcac_dn;
    rcac_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipRootId, mIssuerId);

    ChipLogProgress(Controller, "Generating NOC");
    X509CertRequestParams noc_request = { 1, mNow, mNow + mValidity, noc_dn, icac_dn };
    ReturnErrorOnFailure(NewNodeOperationalX509Cert(noc_request, pubkey, mIntermediateIssuer, noc));

    uint16_t icacBufLen = static_cast<uint16_t>(std::min(icac.size(), static_cast<size_t>(UINT16_MAX)));
    CHIP_ERROR err      = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, icac.data(), icacBufLen));
    if (err == CHIP_NO_ERROR)
    {
        // Found root certificate in the storage.
        icac.reduce_size(icacBufLen);
    }
    else
    {
        ChipLogProgress(Controller, "Generating ICAC");
        X509CertRequestParams icac_request = { 0, mNow, mNow + mValidity, icac_dn, rcac_dn };
        ReturnErrorOnFailure(NewICAX509Cert(icac_request, mIntermediateIssuer.Pubkey(), mIssuer, icac));

        VerifyOrReturnError(CanCastTo<uint16_t>(icac.size()), CHIP_ERROR_INTERNAL);
        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsIntermediateCertificateStorage, key,
                          err = mStorage->SyncSetKeyValue(key, icac.data(), static_cast<uint16_t>(icac.size())));
    }

    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcac.size(), static_cast<size_t>(UINT16_MAX)));
    PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsRootCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, rcac.data(), rcacBufLen));
    if (err == CHIP_NO_ERROR)
    {
        // Found root certificate in the storage.
        rcac.reduce_size(rcacBufLen);
    }
    else
    {
        ChipLogProgress(Controller, "Generating RCAC");
        X509CertRequestParams rcac_request = { 0, mNow, mNow + mValidity, rcac_dn, rcac_dn };
        ReturnErrorOnFailure(NewRootX509Cert(rcac_request, mIssuer, rcac));

        VerifyOrReturnError(CanCastTo<uint16_t>(rcac.size()), CHIP_ERROR_INTERNAL);
        PERSISTENT_KEY_OP(mIndex, kOperationalCredentialsRootCertificateStorage, key,
                          err = mStorage->SyncSetKeyValue(key, rcac.data(), static_cast<uint16_t>(rcac.size())));
    }

    return err;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GenerateNOCChain(const ByteSpan & csrElements,
                                                                 const ByteSpan & attestationSignature, const ByteSpan & DAC,
                                                                 const ByteSpan & PAI, const ByteSpan & PAA,
                                                                 Callback::Callback<OnNOCChainGeneration> * onCompletion)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    NodeId assignedId;
    if (mNodeIdRequested)
    {
        assignedId       = mNextRequestedNodeId;
        mNodeIdRequested = false;
    }
    else
    {
        assignedId = mNextAvailableNodeId++;
    }

    ChipLogProgress(Controller, "Verifying Certificate Signing Request");
    TLVReader reader;
    reader.Init(csrElements);

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag(), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    ReturnErrorCodeIf(!icac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icacSpan(icac.Get(), kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), kMaxCHIPDERCertLength);

    ReturnErrorOnFailure(GenerateNOCChainAfterValidation(assignedId, mNextFabricId, pubkey, rcacSpan, icacSpan, nocSpan));

    ChipLogProgress(Controller, "Providing certificate chain to the commissioner");
    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocSpan, icacSpan, rcacSpan, Optional<AesCcm128KeySpan>(),
                        Optional<NodeId>());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(NodeId * aNodeId)
{
    for (int i = 0; i < 10; ++i)
    {
        CHIP_ERROR err = DRBG_get_bytes(reinterpret_cast<uint8_t *>(aNodeId), sizeof(*aNodeId));
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        if (IsOperationalNodeId(*aNodeId))
        {
            return CHIP_NO_ERROR;
        }
    }

    // Terrible, universe-ending luck (chances are 1 in 2^280 or so here, if our
    // DRBG is good).
    return CHIP_ERROR_INTERNAL;
}

} // namespace Controller
} // namespace chip
