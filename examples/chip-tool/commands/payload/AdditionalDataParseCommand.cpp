/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
