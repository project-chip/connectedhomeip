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

#include "CredentialResponseParser.h"
#include <credentials/CertificationDeclaration.h>
#include <string>

using namespace ::chip;

enum
{
    // Attestation Elements
    kTag_CertDeclaration = 1,
    kTag_Nonce           = 2,
    kTag_TimeStamp       = 3,
    kTag_FirmwareVersion = 4,
    // CSR Elements
    kTag_CSR              = 1,
    kTag_CSRNonce         = 2,
    kTag_vendor_reserved1 = 3,
    kTag_vendor_reserved2 = 4,
    kTag_vendor_reserved3 = 5,
};

CHIP_ERROR AttestationElementsParser::Run()
{
    ChipLogProgress(chipTool, "Parsing Attestation Response Elements.");
    ReturnErrorOnFailure(ParseAttestationElements(mAttResponse));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AttestationElementsParser::ParseAttestationElements(const ByteSpan & responsePayload)
{
    TLV::TLVReader reader;
    TLV::TLVType outerContainer1;

    reader.Init(responsePayload);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer1));

    ByteSpan certDeclaration;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTag_CertDeclaration)));
    ReturnErrorOnFailure(reader.Get(certDeclaration));

    ByteSpan nonce;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTag_Nonce)));
    ReturnErrorOnFailure(reader.Get(nonce));

    uint32_t timeStamp;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTag_TimeStamp)));
    ReturnErrorOnFailure(reader.Get(timeStamp));

    bool fwPresent = false;
    ByteSpan firmwareVersion;
    if (reader.Next(TLV::ContextTag(kTag_FirmwareVersion)) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(firmwareVersion));
        fwPresent = true;
    }

    ByteSpan certDeclarationContent;
    ReturnErrorOnFailure(Credentials::CMS_ExtractCDContent(certDeclaration, certDeclarationContent));

    size_t indent = 0;
    DataModelLogger::LogString("Attestation Response Elements", indent, "{");
    DataModelLogger::LogString("certDeclarationContent", indent + 1, "{");

    Credentials::CertificationElements certElements;
    ReturnErrorOnFailure(DecodeCertificationElements(certDeclarationContent, certElements));
    ReturnErrorOnFailure(DataModelLogger::LogValue("FormatVersion", indent + 2, certElements.FormatVersion));
    ReturnErrorOnFailure(DataModelLogger::LogValue("VendorId", indent + 2, certElements.VendorId));

    std::string arrayString = "";
    DataModelLogger::LogString(indent + 2, "ProductIds = [");
    for (int i = 1; i < certElements.ProductIdsCount + 1; i++)
    {
        arrayString += std::to_string(certElements.ProductIds[i - 1]) + ", ";
        if (i % 10 == 0)
        {
            DataModelLogger::LogString(indent + 5, arrayString);
            arrayString = "";
        }
    }
    DataModelLogger::LogString(indent + 2, "]");

    ReturnErrorOnFailure(DataModelLogger::LogValue("ProductIdsCount", indent + 2, certElements.ProductIdsCount));
    ReturnErrorOnFailure(DataModelLogger::LogValue("DeviceTypeId", indent + 2, certElements.DeviceTypeId));
    DataModelLogger::LogString("CertificateId", indent + 2, std::string(certElements.CertificateId));
    ReturnErrorOnFailure(DataModelLogger::LogValue("SecurityLevel", indent + 2, certElements.SecurityLevel));
    ReturnErrorOnFailure(DataModelLogger::LogValue("SecurityInformation", indent + 2, certElements.SecurityInformation));
    ReturnErrorOnFailure(DataModelLogger::LogValue("VersionNumber", indent + 2, certElements.VersionNumber));
    ReturnErrorOnFailure(DataModelLogger::LogValue("CertificationType", indent + 2, certElements.CertificationType));
    ReturnErrorOnFailure(DataModelLogger::LogValue("DACOriginVendorId (Optional)", indent + 2, certElements.DACOriginVendorId));
    ReturnErrorOnFailure(DataModelLogger::LogValue("DACOriginProductId (Optional)", indent + 2, certElements.DACOriginProductId));
    ReturnErrorOnFailure(
        DataModelLogger::LogValue("DACOriginVIDandPIDPresent (Optional)", indent + 2, certElements.DACOriginVIDandPIDPresent));
    // TODO: Figure out how to log.
    // ReturnErrorOnFailure(DataModelLogger::LogValue("AuthorizedPAAList", indent + 2, certElements.AuthorizedPAAList));
    ReturnErrorOnFailure(DataModelLogger::LogValue("AuthorizedPAAListCount", indent + 2, certElements.AuthorizedPAAListCount));

    DataModelLogger::LogString(indent + 1, "}");
    ReturnErrorOnFailure(DataModelLogger::LogValue("Nonce", indent + 1, nonce));
    ReturnErrorOnFailure(DataModelLogger::LogValue("Time Stamp", indent + 1, timeStamp));
    if (fwPresent)
    {
        ReturnErrorOnFailure(DataModelLogger::LogValue("Firmware Version", indent + 1, firmwareVersion));
    }
    DataModelLogger::LogString(indent, "}");

    return CHIP_NO_ERROR;
}

CHIP_ERROR NOCCSRElementsParser::Run()
{
    // static uint8_t buf[] = {0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x57};
    // ByteSpan test(buf, sizeof(buf));
    ChipLogProgress(chipTool, "Parsing NOC CSR Elements");
    ReturnErrorOnFailure(ParseNOCCSRResponse(mCSRResponse));

    return CHIP_NO_ERROR;
}

CHIP_ERROR NOCCSRElementsParser::ParseNOCCSRResponse(const ByteSpan & responsePayload)
{
    TLV::TLVReader reader;
    TLV::TLVType outerContainer1;

    reader.Init(responsePayload);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer1));

    ByteSpan csr;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTag_CSR)));
    ReturnErrorOnFailure(reader.Get(csr));

    ByteSpan nonce;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTag_CSRNonce)));
    ReturnErrorOnFailure(reader.Get(nonce));

    bool vendorReserved1Present = false;
    ByteSpan vendorReserved1;
    if (reader.Next(TLV::ContextTag(kTag_vendor_reserved1)) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(vendorReserved1));
        vendorReserved1Present = true;
    }

    bool vendorReserved2Present = false;
    ByteSpan vendorReserved2;
    if (reader.Next(TLV::ContextTag(kTag_vendor_reserved2)) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(vendorReserved2));
        vendorReserved2Present = true;
    }

    bool vendorReserved3Present = false;
    ByteSpan vendorReserved3;
    if (reader.Next(TLV::ContextTag(kTag_vendor_reserved3)) == CHIP_NO_ERROR)
    {
        ReturnErrorOnFailure(reader.Get(vendorReserved3));
        vendorReserved3Present = true;
    }

    size_t indent = 0;
    DataModelLogger::LogString("NOC CSR Response Elements", indent, "{");

    ReturnErrorOnFailure(DataModelLogger::LogValue("CSR", indent + 1, csr));
    ReturnErrorOnFailure(DataModelLogger::LogValue("CSR Nonce", indent + 1, nonce));

    if (vendorReserved1Present)
    {
        ReturnErrorOnFailure(DataModelLogger::LogValue("Vendor Reserved 1", indent + 1, vendorReserved1));
    }
    if (vendorReserved2Present)
    {
        ReturnErrorOnFailure(DataModelLogger::LogValue("Vendor Reserved 1", indent + 1, vendorReserved2));
    }
    if (vendorReserved3Present)
    {
        ReturnErrorOnFailure(DataModelLogger::LogValue("Vendor Reserved 1", indent + 1, vendorReserved3));
    }
    DataModelLogger::LogString(indent, "}");

    return CHIP_NO_ERROR;
}