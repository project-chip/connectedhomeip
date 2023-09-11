/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
#include <lib/core/TLVData.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <protocols/Protocols.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>

namespace chip {

CHIP_ERROR AdditionalDataPayloadParser::populatePayload(SetupPayloadData::AdditionalDataPayload & outPayload)
{
    TLV::ContiguousBufferTLVReader reader;
    TLV::ContiguousBufferTLVReader innerReader;

    reader.Init(mPayloadBufferData, mPayloadBufferLength);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    // Open the container
    ReturnErrorOnFailure(reader.OpenContainer(innerReader));
    if (innerReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(SetupPayloadData::kRotatingDeviceIdTag)) == CHIP_NO_ERROR)
    {
        // Get the value of the rotating device id
        ByteSpan rotatingDeviceId;
        ReturnErrorOnFailure(innerReader.GetByteView(rotatingDeviceId));

        VerifyOrReturnError(rotatingDeviceId.size() <= RotatingDeviceId::kMaxLength, CHIP_ERROR_INVALID_STRING_LENGTH);
        char rotatingDeviceIdBufferTemp[RotatingDeviceId::kHexMaxLength];

        ReturnErrorOnFailure(Encoding::BytesToUppercaseHexString(rotatingDeviceId.data(), rotatingDeviceId.size(),
                                                                 rotatingDeviceIdBufferTemp, RotatingDeviceId::kHexMaxLength));
        outPayload.rotatingDeviceId = std::string(rotatingDeviceIdBufferTemp, rotatingDeviceId.size() * 2);
    }
    else
    {
        outPayload.rotatingDeviceId = "";
    }

    // Verify the end of the container
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

} // namespace chip
