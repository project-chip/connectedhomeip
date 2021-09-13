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
#include <math.h>
#include <setup_payload/AdditionalDataPayload.h>
#include <setup_payload/AdditionalDataPayloadParser.h>
#include <string>

using namespace ::chip;
using namespace ::chip::SetupPayloadData;

CHIP_ERROR AdditionalDataParseCommand::Run()
{
    AdditionalDataPayload resultPayload;
    CHIP_ERROR err = CHIP_NO_ERROR;

    size_t additionalDataPayloadBytesLength = static_cast<size_t>(ceil(strlen(mPayload) / 2));
    uint8_t additionalDataPayloadBytes[additionalDataPayloadBytesLength];
    size_t bufferSize =
        chip::Encoding::HexToBytes(mPayload, strlen(mPayload), additionalDataPayloadBytes, additionalDataPayloadBytesLength);

    err = AdditionalDataPayloadParser(additionalDataPayloadBytes, bufferSize).populatePayload(resultPayload);
    SuccessOrExit(err);

    ChipLogProgress(chipTool, "AdditionalDataParseCommand, RotatingDeviceId=%s", resultPayload.rotatingDeviceId.c_str());

exit:
    if (err == CHIP_NO_ERROR)
    {
        SetCommandExitStatus(CHIP_NO_ERROR);
    }
    return err;
}
