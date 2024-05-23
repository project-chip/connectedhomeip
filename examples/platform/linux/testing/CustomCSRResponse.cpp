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

#include "Options.h"

#include <algorithm>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Encode.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <crypto/CHIPCryptoPAL.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::OperationalCredentials::Commands;

constexpr size_t kMaxResponseLength = 900;
constexpr size_t kCSRNonceLength    = 32;

namespace {

CHIP_ERROR ConstructCustomNOCSRElements(TLV::TLVWriter & writer, TLV::Tag tag, const ByteSpan & nocsrElements,
                                        CSRResponseOptions & options)
{
    ByteSpan csr;
    ByteSpan csrNonce;
    ByteSpan vendorReserved1;
    ByteSpan vendorReserved2;
    ByteSpan vendorReserved3;
    ReturnErrorOnFailure(
        Credentials::DeconstructNOCSRElements(nocsrElements, csr, csrNonce, vendorReserved1, vendorReserved2, vendorReserved3));

    // Add 10 bytes of possible overhead to allow the generation of content longer than the allowed maximum of RESP_MAX.
    // 10 has been choosen to leave enough space for the possible TLV overhead when adding the additional data.
    uint8_t nocsrElementsData[kMaxResponseLength + 10];
    MutableByteSpan nocsrElementsSpan(nocsrElementsData);

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(nocsrElementsSpan);

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));

    // Update CSR
    if (options.csrIncorrectType)
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), true));
    }
    else
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), csr));
    }

    // Update CSRNonce
    if (options.csrNonceIncorrectType)
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), true));
    }
    else if (options.csrNonceInvalid)
    {
        uint8_t csrNonceInvalid[kCSRNonceLength] = {};
        memcpy(csrNonceInvalid, csrNonce.data(), csrNonce.size());
        std::reverse(csrNonceInvalid, csrNonceInvalid + sizeof(csrNonceInvalid));
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), ByteSpan(csrNonceInvalid)));
    }
    else if (options.csrNonceTooLong)
    {
        uint8_t csrNonceTooLong[kCSRNonceLength + 1] = {};
        memcpy(csrNonceTooLong, csrNonce.data(), csrNonce.size());
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), ByteSpan(csrNonceTooLong)));
    }
    else
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), csrNonce));
    }

    // Add vendorReserved1 if present
    if (!vendorReserved1.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), vendorReserved1));
    }

    // Add vendorReserved2 if present
    if (!vendorReserved2.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(4), vendorReserved2));
    }

    // Add vendorReserved3 if present
    if (!vendorReserved3.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(5), vendorReserved3));
    }

    // Add additional data
    if (options.nocsrElementsTooLong)
    {
        size_t len = kMaxResponseLength - tlvWriter.GetLengthWritten();
        ReturnLogErrorOnFailure(tlvWriter.Put(TLV::ContextTag(6), ByteSpan(nocsrElementsData, len)));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());

    return DataModel::Encode(writer, tag, nocsrElementsSpan.SubSpan(0, tlvWriter.GetLengthWritten()));
}

CHIP_ERROR ConstructCustomAttestationSignature(TLV::TLVWriter & writer, TLV::Tag tag, const ByteSpan & attestationSignature,
                                               CSRResponseOptions & options)
{
    if (options.attestationSignatureIncorrectType)
    {
        return DataModel::Encode(writer, tag, true);
    }

    if (options.attestationSignatureInvalid)
    {
        uint8_t invalidAttestationSignature[Crypto::kP256_ECDSA_Signature_Length_Raw] = {};
        memcpy(invalidAttestationSignature, attestationSignature.data(), attestationSignature.size());
        std::reverse(invalidAttestationSignature, invalidAttestationSignature + sizeof(invalidAttestationSignature));
        return DataModel::Encode(writer, tag, ByteSpan(invalidAttestationSignature));
    }

    return DataModel::Encode(writer, tag, attestationSignature);
}

} // namespace

namespace chip {
namespace app {
namespace DataModel {

template <>
CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, const CSRResponse::Type & responseData)
{
    auto tag1      = TLV::ContextTag(CSRResponse::Fields::kNOCSRElements);
    auto tag2      = TLV::ContextTag(CSRResponse::Fields::kAttestationSignature);
    auto & options = LinuxDeviceOptions::GetInstance().mCSRResponseOptions;

    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(ConstructCustomNOCSRElements(writer, tag1, responseData.NOCSRElements, options));
    ReturnErrorOnFailure(ConstructCustomAttestationSignature(writer, tag2, responseData.attestationSignature, options));
    ReturnErrorOnFailure(writer.EndContainer(outer));

    return CHIP_NO_ERROR;
}

} // namespace DataModel
} // namespace app
} // namespace chip
