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

#include "AndroidOperationalCredentialsIssuer.h"
#include <algorithm>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>

#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>

namespace chip {
namespace Controller {
constexpr const char kOperationalCredentialsIssuerKeypairStorage[]   = "AndroidDeviceControllerKey";
constexpr const char kOperationalCredentialsRootCertificateStorage[] = "AndroidCARootCert";

using namespace Credentials;
using namespace Crypto;
using namespace TLV;

CHIP_ERROR AndroidOperationalCredentialsIssuer::Initialize(PersistentStorageDelegate & storage, jobject javaObjectRef)
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;

    // Initializing the default start validity to start of 2021. The default validity duration is 10 years.
    CHIP_ZERO_AT(effectiveTime);
    effectiveTime.Year  = 2021;
    effectiveTime.Month = 6;
    effectiveTime.Day   = 10;
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

    mStorage       = &storage;
    mJavaObjectRef = javaObjectRef;

    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidOperationalCredentialsIssuer::GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId,
                                                                                const Crypto::P256PublicKey & pubkey,
                                                                                MutableByteSpan & rcac, MutableByteSpan & icac,
                                                                                MutableByteSpan & noc)
{
    ChipDN noc_dn;
    noc_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipFabricId, fabricId);
    noc_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipNodeId, nodeId);
    ChipDN rcac_dn;
    rcac_dn.AddAttribute(chip::ASN1::kOID_AttributeType_ChipRootId, mIssuerId);

    ChipLogProgress(Controller, "Generating NOC");
    chip::Credentials::X509CertRequestParams noc_request = { 1, mNow, mNow + mValidity, noc_dn, rcac_dn };
    ReturnErrorOnFailure(NewNodeOperationalX509Cert(noc_request, pubkey, mIssuer, noc));
    icac.reduce_size(0);

    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcac.size(), static_cast<size_t>(UINT16_MAX)));
    CHIP_ERROR err      = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(fabricId, kOperationalCredentialsRootCertificateStorage, key,
                      err = mStorage->SyncGetKeyValue(key, rcac.data(), rcacBufLen));
    if (err == CHIP_NO_ERROR)
    {
        // Found root certificate in the storage.
        rcac.reduce_size(rcacBufLen);
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(Controller, "Generating RCAC");
    chip::Credentials::X509CertRequestParams rcac_request = { 0, mNow, mNow + mValidity, rcac_dn, rcac_dn };
    ReturnErrorOnFailure(NewRootX509Cert(rcac_request, mIssuer, rcac));

    VerifyOrReturnError(CanCastTo<uint16_t>(rcac.size()), CHIP_ERROR_INTERNAL);
    PERSISTENT_KEY_OP(fabricId, kOperationalCredentialsRootCertificateStorage, key,
                      err = mStorage->SyncSetKeyValue(key, rcac.data(), static_cast<uint16_t>(rcac.size())));

    return err;
}

CHIP_ERROR AndroidOperationalCredentialsIssuer::GenerateNOCChain(const ByteSpan & csrElements,
                                                                 const ByteSpan & attestationSignature, const ByteSpan & DAC,
                                                                 const ByteSpan & PAI, const ByteSpan & PAA,
                                                                 Callback::Callback<OnNOCChainGeneration> * onCompletion)
{
    jmethodID method;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = JniReferences::GetInstance().FindMethod(JniReferences::GetInstance().GetEnvForCurrentThread(), mJavaObjectRef,
                                                  "onOpCSRGenerationComplete", "([B)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Error invoking onOpCSRGenerationComplete: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

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

    ChipLogProgress(chipTool, "VerifyCertificateSigningRequest");

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), kMaxCHIPDERCertLength);

    MutableByteSpan icacSpan;

    ReturnErrorOnFailure(GenerateNOCChainAfterValidation(assignedId, mNextFabricId, pubkey, rcacSpan, icacSpan, nocSpan));

    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocSpan, ByteSpan(), rcacSpan, Optional<AesCcm128KeySpan>(),
                        Optional<NodeId>());

    jbyteArray javaCsr;
    JniReferences::GetInstance().GetEnvForCurrentThread()->ExceptionClear();
    JniReferences::GetInstance().N2J_ByteArray(JniReferences::GetInstance().GetEnvForCurrentThread(), csrElements.data(),
                                               csrElements.size(), javaCsr);
    JniReferences::GetInstance().GetEnvForCurrentThread()->CallVoidMethod(mJavaObjectRef, method, javaCsr);
    return CHIP_NO_ERROR;
}

} // namespace Controller
} // namespace chip
