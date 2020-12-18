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

    reader.Init(mPayload.data(), (uint32_t) mPayload.size());
    reader.ImplicitProfileId = kProtocol_ServiceProvisioning;
    err                      = reader.Next();
    SuccessOrExit(err);

    // Open the container
    err = reader.OpenContainer(innerReader);
    SuccessOrExit(err);

    err = innerReader.Next();
    SuccessOrExit(err);

    // Get the value of the rotating device id
    if (TagNumFromTag(innerReader.GetTag()) == kRotatingDeviceIdTag)
    {
        char rotatingDeviceId[kRotatingDeviceIdLength];
        err = innerReader.GetString(rotatingDeviceId, sizeof(rotatingDeviceId));
        SuccessOrExit(err);
        outPayload.rotatingDeviceId = std::string(rotatingDeviceId);
    }
    else
    {
        ChipLogError(AdditionalDataPayload, "Unable to identify the included tag, TagFound=%d", innerReader.GetTag());
        return CHIP_ERROR_INVALID_TLV_TAG;
    }

exit:
    return err;
}
