/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "AdditionalDataParseCommand.h"
#include <setup_payload/AdditionalDataPayload.h>
#include <setup_payload/AdditionalDataPayloadParser.h>
#include <string>

using namespace ::chip;
using namespace ::chip::SetupPayloadData;

CHIP_ERROR AdditionalDataParseCommand::Run()
{
    std::vector<uint8_t> payloadData;
    AdditionalDataPayload resultPayload;
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string payloadString(mPayload);

    // Decode input payload
    size_t len = payloadString.length();

    for (size_t i = 0; i < len; i += 2)
    {
        auto str  = payloadString.substr(i, 2);
        uint8_t x = (uint8_t) stoi(str, 0, 16);
        payloadData.push_back(x);
    }

    err = AdditionalDataPayloadParser(payloadData.data(), (uint32_t) payloadData.size()).populatePayload(resultPayload);
    SuccessOrExit(err);

    ChipLogProgress(chipTool, "AdditionalDataParseCommand, RotatingDeviceId=%s", resultPayload.rotatingDeviceId.c_str());

exit:
    if (err == CHIP_NO_ERROR)
    {
        SetCommandExitStatus(CHIP_NO_ERROR);
    }
    return err;
}
