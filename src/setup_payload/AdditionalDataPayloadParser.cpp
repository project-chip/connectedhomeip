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

using namespace chip;
using namespace std;
using namespace chip::SetupPayload;
using namespace chip::TLV;
using namespace chip::TLV::Utilities;
using namespace chip::Protocols;

CHIP_ERROR AdditionalDataPayloadParser::populatePayload(AdditionalDataPayload & outPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVReader reader;
    TLVReader innerReader;

    reader.Init(mPayloadBufferData, mPayloadBufferLength);
    err = reader.Next(kTLVType_Structure, AnonymousTag);
    SuccessOrExit(err);

    // Open the container
    err = reader.OpenContainer(innerReader);
    SuccessOrExit(err);

    err = innerReader.Next(kTLVType_UTF8String, ContextTag(kRotatingDeviceIdTag));
    SuccessOrExit(err);

    // Get the value of the rotating device id
    char rotatingDeviceId[kRotatingDeviceIdLength];
    err = innerReader.GetString(rotatingDeviceId, sizeof(rotatingDeviceId));
    SuccessOrExit(err);
    outPayload.rotatingDeviceId = std::string(rotatingDeviceId);

    // Verify the end of the container
    err = reader.VerifyEndOfContainer();
    SuccessOrExit(err);

exit:
    return err;
}
