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
#include <credentials/CHIPCert.h>
#include <support/CHIPMem.h>
#include <support/ScopedBuffer.h>

namespace chip {
namespace Controller {

constexpr const char kOperationalCredentialsIssuerKeypairStorage[]             = "ExampleOpCredsCAKey";
constexpr const char kOperationalCredentialsIntermediateIssuerKeypairStorage[] = "ExampleOpCredsICAKey";

using namespace Credentials;
using namespace Crypto;

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

    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ExampleOperationalCredentialsIssuer::GenerateNodeOperationalCertificate(const Optional<NodeId> & nodeId, FabricId fabricId,
                                                                        const ByteSpan & csr, const ByteSpan & DAC,
                                                                        Callback::Callback<NOCGenerated> * onNOCGenerated)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    NodeId assignedId;
    if (nodeId.HasValue())
    {
        assignedId = nodeId.Value();
    }
    else
    {
        assignedId = mNextAvailableNodeId++;
    }

    X509CertRequestParams request = { 1, mIntermediateIssuerId, mNow, mNow + mValidity, true, fabricId, true, assignedId };

    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    uint32_t nocLen = 0;

    ReturnErrorOnFailure(NewNodeOperationalX509Cert(request, CertificateIssuerLevel::kIssuerIsIntermediateCA, pubkey,
                                                    mIntermediateIssuer, noc.Get(), kMaxCHIPDERCertLength, nocLen));

    onNOCGenerated->mCall(onNOCGenerated->mContext, ByteSpan(noc.Get(), nocLen));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GetIntermediateCACertificate(FabricId fabricId, MutableByteSpan & outCert)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    X509CertRequestParams request = { 0, mIssuerId, mNow, mNow + mValidity, true, fabricId, false, 0 };

    size_t outCertSize  = (outCert.size() > UINT32_MAX) ? UINT32_MAX : outCert.size();
    uint32_t outCertLen = 0;
    ReturnErrorOnFailure(NewICAX509Cert(request, mIntermediateIssuerId, mIntermediateIssuer.Pubkey(), mIssuer, outCert.data(),
                                        static_cast<uint32_t>(outCertSize), outCertLen));
    outCert.reduce_size(outCertLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleOperationalCredentialsIssuer::GetRootCACertificate(FabricId fabricId, MutableByteSpan & outCert)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    X509CertRequestParams request = { 0, mIssuerId, mNow, mNow + mValidity, true, fabricId, false, 0 };

    size_t outCertSize  = (outCert.size() > UINT32_MAX) ? UINT32_MAX : outCert.size();
    uint32_t outCertLen = 0;
    ReturnErrorOnFailure(NewRootX509Cert(request, mIssuer, outCert.data(), static_cast<uint32_t>(outCertSize), outCertLen));
    outCert.reduce_size(outCertLen);

    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
