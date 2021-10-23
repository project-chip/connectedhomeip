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
using namespace ::chip::Encoding;
using namespace ::chip::SetupPayloadData;

CHIP_ERROR AdditionalDataParseCommand::Run()
{
    VerifyOrReturnError(strlen(mPayload) % 2 == 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    size_t additionalDataPayloadBytesLength = strlen(mPayload) / 2;
    std::unique_ptr<uint8_t[]> additionalDataPayloadBytes(new uint8_t[additionalDataPayloadBytesLength]);

    AdditionalDataPayload resultPayload;
    size_t bufferSize = HexToBytes(mPayload, strlen(mPayload), additionalDataPayloadBytes.get(), additionalDataPayloadBytesLength);
    ReturnErrorOnFailure(AdditionalDataPayloadParser(additionalDataPayloadBytes.get(), bufferSize).populatePayload(resultPayload));

    ChipLogProgress(chipTool, "AdditionalDataParseCommand, RotatingDeviceId=%s", resultPayload.rotatingDeviceId.c_str());

    return CHIP_NO_ERROR;
}
