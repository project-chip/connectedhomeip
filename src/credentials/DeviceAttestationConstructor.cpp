/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "DeviceAttestationConstructor.h"
#include "DeviceAttestationVendorReserved.h"

#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstdint>

namespace chip {
namespace Credentials {

// context tag positions
enum : uint32_t
{
    kCertificationDeclarationTagId = 1,
    kAttestationNonceTagId         = 2,
    kTimestampTagId                = 3,
    kFirmwareInfoTagId             = 4,
};

// utility to determine number of Vendor Reserved elements in a bytespan
CHIP_ERROR CountVendorReservedElementsInDA(const ByteSpan & attestationElements, size_t & numOfElements)
{
    TLV::ContiguousBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    tlvReader.Init(attestationElements);
    ReturnErrorOnFailure(tlvReader.Next(containerType, TLV::AnonymousTag));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    size_t count = 0;
    CHIP_ERROR error;
    while ((error = tlvReader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag = tlvReader.GetTag();
        if (TLV::IsProfileTag(tag))
        {
            count++;
        }
    }
    VerifyOrReturnError(error == CHIP_NO_ERROR || error == CHIP_END_OF_TLV, error);

    numOfElements = count;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeconstructAttestationElements(const ByteSpan & attestationElements, ByteSpan & certificationDeclaration,
                                          ByteSpan & attestationNonce, uint32_t & timestamp, ByteSpan & firmwareInfo,
                                          DeviceAttestationVendorReservedDeconstructor & vendorReserved)
{
    bool certificationDeclarationExists = false;
    bool attestationNonceExists         = false;
    bool timestampExists                = false;
    bool gotFirstContextTag             = false;
    uint32_t lastContextTagId           = 0;

    TLV::ContiguousBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    firmwareInfo = ByteSpan();

    tlvReader.Init(attestationElements);
    ReturnErrorOnFailure(tlvReader.Next(containerType, TLV::AnonymousTag));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    CHIP_ERROR error;

    // process context tags first (should be in sorted order)
    while ((error = tlvReader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = tlvReader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            break;
        }

        // Ensure tag-order and correct first expected tag
        uint32_t contextTagId = TLV::TagNumFromTag(tag);
        if (!gotFirstContextTag)
        {
            // First tag must always be Certification Declaration
            VerifyOrReturnError(contextTagId == kCertificationDeclarationTagId, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
            gotFirstContextTag = true;
        }
        else
        {
            // Subsequent tags must always be in order
            VerifyOrReturnError(contextTagId > lastContextTagId, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
        }
        lastContextTagId = contextTagId;

        switch (contextTagId)
        {
        case kCertificationDeclarationTagId:
            ReturnErrorOnFailure(tlvReader.GetByteView(certificationDeclaration));
            certificationDeclarationExists = true;
            break;
        case kAttestationNonceTagId:
            ReturnErrorOnFailure(tlvReader.GetByteView(attestationNonce));
            attestationNonceExists = true;
            break;
        case kTimestampTagId:
            ReturnErrorOnFailure(tlvReader.Get(timestamp));
            timestampExists = true;
            break;
        case kFirmwareInfoTagId:
            ReturnErrorOnFailure(tlvReader.GetByteView(firmwareInfo));
            break;
        default:
            // It's OK to have future context tags before vendor specific tags.
            // We already checked that the tags are in order.
            break;
        }
    }

    VerifyOrReturnError(error == CHIP_NO_ERROR || error == CHIP_END_OF_TLV, error);

    const bool allTagsNeededPresent = certificationDeclarationExists && attestationNonceExists && timestampExists;
    VerifyOrReturnError(allTagsNeededPresent, CHIP_ERROR_MISSING_TLV_ELEMENT);

    size_t count = 0;
    ReturnErrorOnFailure(CountVendorReservedElementsInDA(attestationElements, count));
    ReturnErrorOnFailure(vendorReserved.PrepareToReadVendorReservedElements(attestationElements, count));
    return CHIP_NO_ERROR;
}

// Have a class for vendor reserved data, discussed in:
// https://github.com/project-chip/connectedhomeip/issues/9825
CHIP_ERROR ConstructAttestationElements(const ByteSpan & certificationDeclaration, const ByteSpan & attestationNonce,
                                        uint32_t timestamp, const ByteSpan & firmwareInfo,
                                        DeviceAttestationVendorReservedConstructor & vendorReserved,
                                        MutableByteSpan & attestationElements)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    VerifyOrReturnError(!certificationDeclaration.empty() && !attestationNonce.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(attestationNonce.size() == 32, CHIP_ERROR_INVALID_ARGUMENT);

    tlvWriter.Init(attestationElements.data(), static_cast<uint32_t>(attestationElements.size()));
    outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), certificationDeclaration));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), attestationNonce));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), timestamp));
    if (!firmwareInfo.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(4), firmwareInfo));
    }

    const VendorReservedElement * element = vendorReserved.cbegin();
    while ((element = vendorReserved.Next()) != nullptr)
    {
        ReturnErrorOnFailure(
            tlvWriter.Put(TLV::ProfileTag(element->vendorId, element->profileNum, element->tagNum), element->vendorReservedData));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    attestationElements = attestationElements.SubSpan(0, tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

} // namespace Credentials

} // namespace chip
