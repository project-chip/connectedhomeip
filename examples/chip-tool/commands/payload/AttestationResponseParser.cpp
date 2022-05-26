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

#include "AttestationResponseParser.h"
#include <credentials/CertificationDeclaration.h>
#include <string>


using namespace ::chip;

enum
{
    kTag_CertDecleration       = 1,
    kTag_Signature             = 2,
};

CHIP_ERROR AttestationResponseParser::Run()
{
    ChipLogProgress(chipTool, "Parsing Attestation Response.");
    ParseAttestationResponse(mAttResponse);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AttestationResponseParser::ParseAttestationResponse(chip::ByteSpan & responsePayload)
{
    chip::Credentials::CertificationElements certElements;
    chip::ByteSpan certDecleration;
    chip::ByteSpan signature;
    chip::ByteSpan certDeclerationElements;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVType outerContainer1;
    size_t indent = 0;

    reader.Init(responsePayload);
    ReturnErrorOnFailure(reader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer1));

    ReturnErrorOnFailure(reader.Next(chip::TLV::ContextTag(kTag_CertDecleration)));
    ReturnErrorOnFailure(reader.Get(certDecleration));

    ReturnErrorOnFailure(reader.Next(chip::TLV::ContextTag(kTag_Signature)));
    ReturnErrorOnFailure(reader.Get(signature));

    ReturnErrorOnFailure(chip::Credentials::CMS_ExtractCDContent(certDecleration, certDeclerationElements));

    DataModelLogger::LogString("AttestationResponse", indent, "{");
    DataModelLogger::LogString("certDeclerationElements", indent + 1, "{");

    ReturnErrorOnFailure(DecodeCertificationElements(certDeclerationElements, certElements));
    ReturnErrorOnFailure(DataModelLogger::LogValue("FormatVersion", indent + 2, certElements.FormatVersion));
    ReturnErrorOnFailure(DataModelLogger::LogValue("VendorId", indent + 2, certElements.VendorId));

    std::string arrayString = "";
    DataModelLogger::LogString(indent + 2, "ProductIds = [");
    for (int i = 1; i < certElements.ProductIdsCount + 1; i++) {
        arrayString += std::to_string(certElements.ProductIds[i - 1]) + ", ";
        if (i % 10 == 0) {
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
    ReturnErrorOnFailure(DataModelLogger::LogValue("CertificationType", indent + 2, certElements.CertificationType));
    ReturnErrorOnFailure(DataModelLogger::LogValue("DACOriginVendorId (Optional)", indent + 2, certElements.DACOriginVendorId));
    ReturnErrorOnFailure(DataModelLogger::LogValue("DACOriginProductId (Optional)", indent + 2, certElements.DACOriginProductId));
    ReturnErrorOnFailure(DataModelLogger::LogValue("DACOriginVIDandPIDPresent (Optional)", indent + 2, certElements.DACOriginVIDandPIDPresent));
    // TODO: Figure out how to log.
    // ReturnErrorOnFailure(DataModelLogger::LogValue("AuthorizedPAAList", indent + 2, certElements.AuthorizedPAAList));
    ReturnErrorOnFailure(DataModelLogger::LogValue("AuthorizedPAAListCount", indent + 2, certElements.AuthorizedPAAListCount));

    DataModelLogger::LogString(indent + 1, "}");
    ReturnErrorOnFailure(DataModelLogger::LogValue("signature", indent + 1, signature));
    DataModelLogger::LogString(indent, "}");

    return CHIP_NO_ERROR;
}
