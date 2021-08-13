/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <core/CHIPTLV.h>
#include <credentials/CHIPCert.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/PersistentStorageMacros.h>
#include <support/SafeInt.h>
#include <support/ScopedBuffer.h>

namespace chip {
namespace Controller {

constexpr const char kOperationalCredentialsIssuerKeypairStorage[]             = "ExampleOpCredsCAKey";
constexpr const char kOperationalCredentialsIntermediateIssuerKeypairStorage[] = "ExampleOpCredsICAKey";
constexpr const char kOperationalCredentialsRootCertificateStorage[]           = "ExampleCARootCert";

using namespace Credentials;
using namespace Crypto;
using namespace TLV;

CHIP_ERROR ExampleOperationalCredentialsIssuer::Initialize(PersistentStorageDelegate & storage)
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;

    // Initializing the default start validity to start of 2021. The default validity duration is 10 years.
    CHIP_ZERO_AT(effectiveTime);
    effectiveTime.Year  = 2021;
    effectiveTime.Month = 1;
    effectiveTime.Day   = 1;
    ReturnErrorOnFailure(ASN1ToChipEpochTime(effectiveTime, mNow));

    Crypto::P256SerializedKeypair serializedKey;
    uint16_t keySize = static_cast<uint16_t>(sizeof(serializedKey));

    if (storage.SyncGetKeyValue(kOperationalCredentialsIssuerKeypairStorage, &serializedKey, keySize) != CHIP_NO_ERROR)
    {
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIssuer.Initialize());
        ReturnErrorOnFailure(mIssuer.Serialize(serializedKey));

        keySize = static_cast<uint16_t>(sizeof(serializedKey));
        ReturnErrorOnFailure(storage.SyncSetKeyValue(kOperationalCredentialsIssuerKeypairStorage, &serializedKey, keySize));
    }
    else
    {
        // Use the keypair from the storage
        ReturnErrorOnFailure(mIssuer.Deserialize(serializedKey));
    }

    keySize = static_cast<uint16_t>(sizeof(serializedKey));

    if (storage.SyncGetKeyValue(kOperationalCredentialsIntermediateIssuerKeypairStorage, &serializedKey, keySize) != CHIP_NO_ERROR)
    {
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        ReturnErrorOnFailure(mIntermediateIssuer.Initialize());
        ReturnErrorOnFailure(mIntermediateIssuer.Serialize(serializedKey));

        keySize = static_cast<uint16_t>(sizeof(serializedKey));
        ReturnErrorOnFailure(
            storage.SyncSetKeyValue(kOperationalCredentialsIntermediateIssuerKeypairStorage, &serializedKey, keySize));
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

    X509CertRequestParams noc_request = { 1, mIntermediateIssuerId, mNow, mNow + mValidity, true, mNextFabricId, true, assignedId };

    ChipLogProgress(Controller, "Verifying Certificate Signing Request");
    TLVReader reader;
    reader.Init(csrElements.data(), csrElements.size());

    if (reader.GetType() == kTLVType_NotSpecified)
    {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocCert(noc.Get(), kMaxCHIPDERCertLength);

    ChipLogProgress(Controller, "Generating NOC");
    ReturnErrorOnFailure(NewNodeOperationalX509Cert(noc_request, CertificateIssuerLevel::kIssuerIsIntermediateCA, pubkey,
                                                    mIntermediateIssuer, nocCert));

    X509CertRequestParams icac_request = { 0, mIssuerId, mNow, mNow + mValidity, true, mNextFabricId, false, 0 };

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    ReturnErrorCodeIf(!icac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icaCert(icac.Get(), kMaxCHIPDERCertLength);

    ChipLogProgress(Controller, "Generating ICAC");
    ReturnErrorOnFailure(NewICAX509Cert(icac_request, mIntermediateIssuerId, mIntermediateIssuer.Pubkey(), mIssuer, icaCert));

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rootCert(rcac.Get(), kMaxCHIPDERCertLength);
    uint16_t rootCertBufLen = kMaxCHIPDERCertLength;

    CHIP_ERROR err = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(mNextFabricId, kOperationalCredentialsRootCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, rootCert.data(), rootCertBufLen));
    if (err != CHIP_NO_ERROR)
    {
        // Storage doesn't have an existing root certificate. Let's create one and add it to the storage.
        X509CertRequestParams request = { 0, mIssuerId, mNow, mNow + mValidity, true, mNextFabricId, false, 0 };
        ChipLogProgress(Controller, "Generating RCAC");
        ReturnErrorOnFailure(NewRootX509Cert(request, mIssuer, rootCert));

        VerifyOrReturnError(CanCastTo<uint16_t>(rootCert.size()), CHIP_ERROR_INVALID_ARGUMENT);
        PERSISTENT_KEY_OP(mNextFabricId, kOperationalCredentialsRootCertificateStorage, key,
                          err = mStorage->SyncSetKeyValue(key, rootCert.data(), static_cast<uint16_t>(rootCert.size())));
        ReturnErrorOnFailure(err);
    }
    else
    {
        rootCert.reduce_size(rootCertBufLen);
    }

    ChipLogProgress(Controller, "Providing certificate chain to the commissioner");
    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocCert, icaCert, rootCert);
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
