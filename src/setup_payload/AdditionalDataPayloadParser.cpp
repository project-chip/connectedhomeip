/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file describes a AdditionalData Payload parser based on the
 *      CHIP specification.
 */

#include "AdditionalDataPayloadParser.h"

#include <cstdlib>
#include <string.h>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLVData.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CodeUtils.h>
#include <protocols/Protocols.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>

namespace chip {

CHIP_ERROR AdditionalDataPayloadParser::populatePayload(SetupPayloadData::AdditionalDataPayload & outPayload)
{
    TLV::ContiguousBufferTLVReader reader;
    TLV::ContiguousBufferTLVReader innerReader;

    reader.Init(mPayloadBufferData, mPayloadBufferLength);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag));

    // Open the container
    ReturnErrorOnFailure(reader.OpenContainer(innerReader));

    ReturnErrorOnFailure(innerReader.Next(TLV::kTLVType_UTF8String, TLV::ContextTag(SetupPayloadData::kRotatingDeviceIdTag)));

    // Get the value of the rotating device id
    Span<const char> rotatingDeviceId;
    ReturnErrorOnFailure(innerReader.GetStringView(rotatingDeviceId));

    // This test uses <, not <=, because kHexMaxLength includes the null-terminator.
    VerifyOrReturnError(rotatingDeviceId.size() < RotatingDeviceId::kHexMaxLength, CHIP_ERROR_INVALID_STRING_LENGTH);
    outPayload.rotatingDeviceId = std::string(rotatingDeviceId.data(), rotatingDeviceId.size());

    // Verify the end of the container
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

} // namespace chip
