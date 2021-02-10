/* See Project CHIP LICENSE file for licensing information. */

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    TLV::TLVReader innerReader;

    reader.Init(mPayloadBufferData, mPayloadBufferLength);
    err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag);
    SuccessOrExit(err);

    // Open the container
    err = reader.OpenContainer(innerReader);
    SuccessOrExit(err);

    err = innerReader.Next(TLV::kTLVType_UTF8String, TLV::ContextTag(SetupPayloadData::kRotatingDeviceIdTag));
    SuccessOrExit(err);

    // Get the value of the rotating device id
    char rotatingDeviceId[SetupPayloadData::kRotatingDeviceIdLength];
    err = innerReader.GetString(rotatingDeviceId, sizeof(rotatingDeviceId));
    SuccessOrExit(err);
    outPayload.rotatingDeviceId = std::string(rotatingDeviceId);

    // Verify the end of the container
    err = reader.VerifyEndOfContainer();
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace chip
