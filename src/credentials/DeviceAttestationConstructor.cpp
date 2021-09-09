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

#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <iostream>

namespace chip {
namespace Credentials {

// context tag positions
enum
{
    CERTIFICATION_DECLARATION = 1,
    ATTESTATION_NONCE         = 2,
    TIMESTAMP                 = 3,
    FIRMWARE_INFO             = 4,
    NUMBER_OF_VALID_TAGS      = FIRMWARE_INFO,
};

CHIP_ERROR DeconstructAttestationElements(const ByteSpan & attestationElements, ByteSpan & certificationDeclaration,
                                          ByteSpan & attestationNonce, uint32_t & timestamp, ByteSpan & firmwareInfo,
                                          std::vector<ByteSpan> & vendorReserved, uint16_t & vendorId, uint16_t & profileNum)
{
    CHIP_ERROR error                         = CHIP_NO_ERROR;
    uint32_t currentDecodeTagId              = 0;
    bool argExists[NUMBER_OF_VALID_TAGS + 1] = { false }; // the 0 element is not used so it aligns with tag numbers
    TLV::TLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    vendorReserved.clear();

    tlvReader.Init(attestationElements.data(), static_cast<uint32_t>(attestationElements.size()));
    ReturnErrorOnFailure(tlvReader.Next(containerType, TLV::AnonymousTag));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    while ((error = tlvReader.Next()) == CHIP_NO_ERROR)
    {
        uint64_t tag;
        tag = tlvReader.GetTag();

        currentDecodeTagId = TLV::TagNumFromTag(tag);
        VerifyOrReturnError(currentDecodeTagId != 0, CHIP_ERROR_INVALID_TLV_TAG);

        if (TLV::IsContextTag(tag))
        {
            VerifyOrReturnError(currentDecodeTagId <= NUMBER_OF_VALID_TAGS, CHIP_ERROR_INVALID_TLV_TAG);

            if (argExists[currentDecodeTagId])
            {
                ChipLogProgress(Zcl, "Duplicate TLV tag %" PRIx32, TLV::TagNumFromTag(tag));
                return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT;
            }
            else
            {
                argExists[currentDecodeTagId] = true;
            }

            switch (currentDecodeTagId)
            {
            case CERTIFICATION_DECLARATION:
                ReturnErrorOnFailure(tlvReader.Get(certificationDeclaration));
                break;
            case ATTESTATION_NONCE:
                ReturnErrorOnFailure(tlvReader.Get(attestationNonce));
                break;
            case TIMESTAMP:
                ReturnErrorOnFailure(tlvReader.Get(timestamp));
                break;
            case FIRMWARE_INFO:
                ReturnErrorOnFailure(tlvReader.Get(firmwareInfo));
                break;
            default:
                return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT;
            }
        }
        else if (TLV::IsProfileTag(tag))
        {
            // vendor fields
            bool seenProfile = false;
            uint16_t currentVendorId;
            uint16_t currentProfileNum;

            currentVendorId   = TLV::VendorIdFromTag(tag);
            currentProfileNum = TLV::ProfileNumFromTag(tag);
            if (false == seenProfile)
            {
                seenProfile = true;
                vendorId    = currentVendorId;
                profileNum  = currentProfileNum;
            }
            else
            {
                // check that vendorId and profileNum match in every Vendor Reserved entry
                VerifyOrReturnError(currentVendorId == vendorId && currentProfileNum == profileNum,
                                    CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT);
            }

            ByteSpan vendorReservedEntry;
            ReturnErrorOnFailure(tlvReader.Get(vendorReservedEntry));
            vendorReserved.push_back(vendorReservedEntry);
        }
        else
        {
            return CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT;
        }
    }

    VerifyOrReturnError(error == CHIP_END_OF_TLV, error);
    VerifyOrReturnError(argExists[CERTIFICATION_DECLARATION] && argExists[ATTESTATION_NONCE] && argExists[TIMESTAMP],
                        CHIP_ERROR_MISSING_TLV_ELEMENT);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConstructAttestationElements(const ByteSpan & certificationDeclaration, const ByteSpan & attestationNonce,
                                        uint32_t timestamp, const ByteSpan & firmwareInfo, std::vector<ByteSpan> & vendorReserved,
                                        uint16_t vendorId, uint16_t profileNum, MutableByteSpan & attestationElements)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    VerifyOrReturnError(!certificationDeclaration.empty() && !attestationNonce.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(attestationNonce.size() == 32, CHIP_ERROR_INVALID_MESSAGE_LENGTH);

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

    uint8_t vendorTagNum = 1;
    for (auto & vendorItem : vendorReserved)
    {
        if (!vendorItem.empty())
        {
            ReturnErrorOnFailure(tlvWriter.Put(TLV::ProfileTag(vendorId, profileNum, vendorTagNum), vendorItem));
        }
        vendorTagNum++;
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    attestationElements = attestationElements.SubSpan(0, tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

} // namespace Credentials

} // namespace chip
