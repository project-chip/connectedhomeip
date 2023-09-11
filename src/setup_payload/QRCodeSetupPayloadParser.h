/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file describes a QRCode Setup Payload parser based on the
 *      CHIP specification.
 */

#pragma once

#include "SetupPayload.h"

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

#include <string>
#include <utility>

namespace chip {

/**
 * @class QRCodeSetupPayloadParser
 * A class that can be used to convert a base38 encoded payload to a SetupPayload object
 * */
class QRCodeSetupPayloadParser
{
private:
    std::string mBase38Representation;

public:
    QRCodeSetupPayloadParser(std::string base38Representation) : mBase38Representation(std::move(base38Representation)) {}
    CHIP_ERROR populatePayload(SetupPayload & outPayload);
    static std::string ExtractPayload(std::string inString);

private:
    CHIP_ERROR retrieveOptionalInfos(SetupPayload & outPayload, TLV::ContiguousBufferTLVReader & reader);
    CHIP_ERROR populateTLV(SetupPayload & outPayload, const std::vector<uint8_t> & buf, size_t & index);
    CHIP_ERROR parseTLVFields(chip::SetupPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes);
};

} // namespace chip
