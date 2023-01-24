/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "DecoderCustomLog.h"

#include "../logging/Log.h"

#include <app/AttributeAccessInterface.h>
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/MessageDef/ReportDataMessage.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationVendorReserved.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>

#include <lib/core/TLV.h>
#include <lib/support/TypeTraits.h>

namespace {
template <typename T>
bool IsTag(const chip::TLV::TLVReader & reader, T tag)
{
    return chip::to_underlying(tag) == chip::TLV::TagNumFromTag(reader.GetTag());
}
} // namespace

namespace chip {
namespace trace {
namespace interaction_model {

using namespace logging;

CHIP_ERROR LogCertificateChainResponse(TLV::TLVReader & reader)
{
    app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType value;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));

    LogCertificate("DAC/PAI", value.certificate);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogCertificationDeclaration(const ByteSpan & cd)
{
    auto scopedIndent = ScopedLogIndent("Certification Declaration");

    // TODO Add an option to load a TrustStore so the subjectKeyId can be extracted from the CMS envelope in order
    // to select the proper public key.
    ByteSpan kid;
    ReturnErrorOnFailure(Credentials::CMS_ExtractKeyId(cd, kid));

    Crypto::P256PublicKey verifyingKey;
    Credentials::CsaCdKeysTrustStore cdKeysTrustStore;
    ReturnErrorOnFailure(cdKeysTrustStore.LookupVerifyingKey(kid, verifyingKey));

    ByteSpan cdContentOut;
    ReturnErrorOnFailure(Credentials::CMS_Verify(cd, verifyingKey, cdContentOut));

    constexpr uint8_t kTag_FormatVersion       = 0;  /**< [ unsigned int ] Format version. */
    constexpr uint8_t kTag_VendorId            = 1;  /**< [ unsigned int ] Vedor identifier. */
    constexpr uint8_t kTag_ProductIdArray      = 2;  /**< [ array ] Product identifiers (each is unsigned int). */
    constexpr uint8_t kTag_DeviceTypeId        = 3;  /**< [ unsigned int ] Device Type identifier. */
    constexpr uint8_t kTag_CertificateId       = 4;  /**< [ UTF-8 string, length 19 ] Certificate identifier. */
    constexpr uint8_t kTag_SecurityLevel       = 5;  /**< [ unsigned int ] Security level. */
    constexpr uint8_t kTag_SecurityInformation = 6;  /**< [ unsigned int ] Security information. */
    constexpr uint8_t kTag_VersionNumber       = 7;  /**< [ unsigned int ] Version number. */
    constexpr uint8_t kTag_CertificationType   = 8;  /**< [ unsigned int ] Certification Type. */
    constexpr uint8_t kTag_DACOriginVendorId   = 9;  /**< [ unsigned int, optional ] DAC origin vendor identifier. */
    constexpr uint8_t kTag_DACOriginProductId  = 10; /**< [ unsigned int, optional ] DAC origin product identifier. */
    constexpr uint8_t kTag_AuthorizedPAAList   = 11; /**< [ array, optional ] Authorized PAA List. */

    TLV::TLVReader reader;
    reader.Init(cdContentOut);

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    uint16_t value;
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        switch (TLV::TagNumFromTag(reader.GetTag()))
        {
        case kTag_FormatVersion:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Format Version", value);
            break;
        case kTag_VendorId:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Vendor Id", value);
            break;
        case kTag_ProductIdArray: {
            TLV::TLVType arrayType = TLV::kTLVType_Array;
            ReturnErrorOnFailure(reader.EnterContainer(arrayType));
            auto scopedProductIndent = ScopedLogIndent("Product Ids");
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                ReturnErrorOnFailure(reader.Get(value));
                Log("Product Id", value);
            }
            ReturnErrorOnFailure(reader.ExitContainer(arrayType));
            break;
        }
        case kTag_DeviceTypeId:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Device Type Id", value);
            break;
        case kTag_CertificateId: {
            CharSpan str;
            ReturnErrorOnFailure(reader.Get(str));
            Log("Certificate Id", str);
            break;
        }
        case kTag_SecurityLevel:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Security Level", value);
            break;
        case kTag_SecurityInformation:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Security Information", value);
            break;
        case kTag_VersionNumber:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Version Number", value);
            break;
        case kTag_CertificationType:
            ReturnErrorOnFailure(reader.Get(value));
            Log("Certification Type", value);
            break;
        case kTag_DACOriginVendorId:
            ReturnErrorOnFailure(reader.Get(value));
            Log("DAC Origin Vendor Id", value);
            break;
        case kTag_DACOriginProductId:
            ReturnErrorOnFailure(reader.Get(value));
            Log("DAC Origin Product Id", value);
            break;
        case kTag_AuthorizedPAAList: {
            TLV::TLVType arrayType = TLV::kTLVType_Array;
            ReturnErrorOnFailure(reader.EnterContainer(arrayType));
            auto scopedAuthorizedPAAIndent = ScopedLogIndent("Authorized PAAs");
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                ByteSpan str;
                ReturnErrorOnFailure(reader.Get(str));
                Log("Product Id", str);
            }
            ReturnErrorOnFailure(reader.ExitContainer(arrayType));
            break;
        }
        default:
            Log("<Unsupported Tag>");
            break;
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogAttestationResponse(TLV::TLVReader & reader)
{
    app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType value;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));

    ByteSpan certificationDeclaration;
    ByteSpan attestationNonce;
    uint32_t timestamp;
    ByteSpan firmwareInfo;
    Credentials::DeviceAttestationVendorReservedDeconstructor vendorReserved;

    ReturnErrorOnFailure(Credentials::DeconstructAttestationElements(value.attestationElements, certificationDeclaration,
                                                                     attestationNonce, timestamp, firmwareInfo, vendorReserved));
    auto scopedIndent = ScopedLogIndentWithSize("Attestation Elements", value.attestationElements.size());
    ReturnErrorOnFailure(LogCertificationDeclaration(certificationDeclaration));
    Log("Attestation Nonce", attestationNonce);
    Log("Timestamp", timestamp);
    Log("Firmware Info", firmwareInfo);
    if (vendorReserved.GetNumberOfElements())
    {
        auto scopedVendorIndent = ScopedLogIndent("Vendors Reserved");
        Credentials::VendorReservedElement element;
        while (vendorReserved.GetNextVendorReservedElement(element) == CHIP_NO_ERROR)
        {
            auto scopedVendorReservedIndent = ScopedLogIndent("Vendor Reserved");
            Log("Vendor Id", element.vendorId);
            Log("Profile Number", element.profileNum);
            Log("Profile Data", element.vendorReservedData);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogCSRResponse(TLV::TLVReader & reader)
{
    app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType value;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));

    ByteSpan csr;
    ByteSpan csrNonce;
    ByteSpan vendorReserved1;
    ByteSpan vendorReserved2;
    ByteSpan vendorReserved3;

    ReturnErrorOnFailure(Credentials::DeconstructNOCSRElements(value.NOCSRElements, csr, csrNonce, vendorReserved1, vendorReserved2,
                                                               vendorReserved3));

    {
        auto scopedIndent = ScopedLogIndent("NOCSR Elements");
        Log("CSR", csr);
        Log("CSRNonce", csrNonce);
        Log("Vendor Reserved 1", vendorReserved1);
        Log("Vendor Reserved 2", vendorReserved2);
        Log("Vendor Reserved 3", vendorReserved3);
    }

    LogCertificateRequest("CSR", csr);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogAddNOC(TLV::TLVReader & reader)
{
    app::Clusters::OperationalCredentials::Commands::AddNOC::DecodableType value;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));

    LogCertificate("NOCValue", value.NOCValue);

    if (value.ICACValue.HasValue())
    {
        LogCertificate("ICACValue", value.ICACValue.Value());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogAddTrustedRootCertificateRequest(TLV::TLVReader & reader)
{
    app::Clusters::OperationalCredentials::Commands::AddTrustedRootCertificate::DecodableType value;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));

    LogCertificate("RCACValue", value.rootCACertificate);

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogNOCsAttribute(TLV::TLVReader & reader)
{
    if (reader.GetType() == TLV::kTLVType_Array)
    {
        return CHIP_NO_ERROR;
    }

    chip::app::Clusters::OperationalCredentials::Structs::NOCStruct::DecodableType value;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));

    LogCertificate("NOC", value.noc);

    if (!value.icac.IsNull())
    {
        LogCertificate("ICAC", value.icac.Value());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MaybeDecodeAttributeData(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ClusterId clusterId;
    AttributeId attributeId;

    TLV::TLVType containerType;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (IsTag(reader, app::AttributeDataIB::Tag::kPath))
        {
            ReturnErrorOnFailure(reader.EnterContainer(containerType));
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                if (to_underlying(app::AttributePathIB::Tag::kCluster) == TLV::TagNumFromTag(reader.GetTag()))
                {
                    reader.Get(clusterId);
                }
                else if (to_underlying(app::AttributePathIB::Tag::kAttribute) == TLV::TagNumFromTag(reader.GetTag()))
                {
                    reader.Get(attributeId);
                }
            }
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
        }
        else if (IsTag(reader, app::AttributeDataIB::Tag::kData))
        {
            switch (clusterId)
            {
            case app::Clusters::OperationalCredentials::Id:
                switch (attributeId)
                {
                case app::Clusters::OperationalCredentials::Attributes::NOCs::Id:
                    return LogNOCsAttribute(reader);
                default:
                    return CHIP_NO_ERROR;
                }
            default:
                return CHIP_NO_ERROR;
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MaybeDecodeCommandData(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ClusterId clusterId;
    CommandId commandId;

    TLV::TLVType containerType;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (IsTag(reader, app::CommandDataIB::Tag::kPath))
        {
            ReturnErrorOnFailure(reader.EnterContainer(containerType));
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                if (to_underlying(app::CommandPathIB::Tag::kClusterId) == TLV::TagNumFromTag(reader.GetTag()))
                {
                    reader.Get(clusterId);
                }
                else if (to_underlying(app::CommandPathIB::Tag::kCommandId) == TLV::TagNumFromTag(reader.GetTag()))
                {
                    reader.Get(commandId);
                }
            }
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
        }
        else if (IsTag(reader, app::CommandDataIB::Tag::kFields))
        {
            switch (clusterId)
            {
            case app::Clusters::OperationalCredentials::Id:
                switch (commandId)
                {
                case app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::Id:
                    return LogCertificateChainResponse(reader);
                case app::Clusters::OperationalCredentials::Commands::AttestationResponse::Id:
                    return LogAttestationResponse(reader);
                case app::Clusters::OperationalCredentials::Commands::CSRResponse::Id:
                    return LogCSRResponse(reader);
                case app::Clusters::OperationalCredentials::Commands::AddNOC::Id:
                    return LogAddNOC(reader);
                case app::Clusters::OperationalCredentials::Commands::AddTrustedRootCertificate::Id:
                    return LogAddTrustedRootCertificateRequest(reader);
                default:
                    return CHIP_NO_ERROR;
                }
                break;
            default:
                break;
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MaybeDecodeNestedReadResponse(const uint8_t * data, size_t dataLen)
{
    TLV::TLVReader reader;
    reader.Init(data, dataLen);

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (IsTag(reader, app::ReportDataMessage::Tag::kAttributeReportIBs))
        {
            ReturnErrorOnFailure(reader.EnterContainer(containerType));
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                TLV::TLVType arrayType = TLV::kTLVType_Array;
                ReturnErrorOnFailure(reader.EnterContainer(arrayType));
                while (CHIP_NO_ERROR == (err = reader.Next()))
                {
                    if (IsTag(reader, app::AttributeReportIB::Tag::kAttributeData))
                    {
                        ReturnErrorOnFailure(reader.EnterContainer(containerType));
                        ReturnErrorOnFailure(MaybeDecodeAttributeData(reader));
                        ReturnErrorOnFailure(reader.ExitContainer(containerType));
                    }
                }
                ReturnErrorOnFailure(reader.ExitContainer(arrayType));
            }
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
        }
    }

    return reader.ExitContainer(containerType);
}

CHIP_ERROR MaybeDecodeNestedCommandResponse(const uint8_t * data, size_t dataLen)
{
    TLV::TLVReader reader;
    reader.Init(data, dataLen);

    ReturnErrorOnFailure(reader.Next());

    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (IsTag(reader, app::InvokeResponseMessage::Tag::kInvokeResponses))
        {
            ReturnErrorOnFailure(reader.EnterContainer(containerType));
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                ReturnErrorOnFailure(reader.EnterContainer(containerType));
                while (CHIP_NO_ERROR == (err = reader.Next()))
                {
                    if (IsTag(reader, app::InvokeResponseIB::Tag::kCommand))
                    {
                        ReturnErrorOnFailure(reader.EnterContainer(containerType));
                        ReturnErrorOnFailure(MaybeDecodeCommandData(reader));
                        ReturnErrorOnFailure(reader.ExitContainer(containerType));
                    }
                }
                ReturnErrorOnFailure(reader.ExitContainer(containerType));
            }
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
        }
    }

    return reader.ExitContainer(containerType);
}

CHIP_ERROR MaybeDecodeNestedCommandRequest(const uint8_t * data, size_t dataLen)
{
    TLV::TLVReader reader;
    reader.Init(data, dataLen);

    ReturnErrorOnFailure(reader.Next());

    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (IsTag(reader, app::InvokeRequestMessage::Tag::kInvokeRequests))
        {
            ReturnErrorOnFailure(reader.EnterContainer(containerType));
            while (CHIP_NO_ERROR == (err = reader.Next()))
            {
                ReturnErrorOnFailure(reader.EnterContainer(containerType));
                ReturnErrorOnFailure(MaybeDecodeCommandData(reader));
                ReturnErrorOnFailure(reader.ExitContainer(containerType));
            }
            ReturnErrorOnFailure(reader.ExitContainer(containerType));
        }
    }

    return reader.ExitContainer(containerType);
}

} // namespace interaction_model
} // namespace trace
} // namespace chip
