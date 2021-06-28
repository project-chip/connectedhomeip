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

#include <core/CHIPError.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>

namespace chip {

CHIP_ERROR AdditionalDataPayloadParser::populatePayload(SetupPayloadData::AdditionalDataPayload & outPayload)
{
    TLV::TLVReader reader;
    TLV::TLVReader innerReader;

    reader.Init(mPayloadBufferData, mPayloadBufferLength);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag));

    // Open the container
    ReturnErrorOnFailure(reader.OpenContainer(innerReader));

    ReturnErrorOnFailure(innerReader.Next(TLV::kTLVType_UTF8String, TLV::ContextTag(SetupPayloadData::kRotatingDeviceIdTag)));

    // Get the value of the rotating device id
    char rotatingDeviceId[SetupPayloadData::kRotatingDeviceIdLength];
    ReturnErrorOnFailure(innerReader.GetString(rotatingDeviceId, sizeof(rotatingDeviceId)));
    outPayload.rotatingDeviceId = std::string(rotatingDeviceId);

    // Verify the end of the container
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

} // namespace chip
