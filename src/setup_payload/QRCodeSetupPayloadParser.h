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
    /**
     * base38Representation is expected to start with the "MT:" prefix, in general.
     */
    QRCodeSetupPayloadParser(std::string base38Representation) : mBase38Representation(std::move(base38Representation)) {}

    /**
     * Populate the payload based on the provided base38 representation.  If the
     * provided representation contains concatenated payloads, this method will
     * fail.
     */
    CHIP_ERROR populatePayload(SetupPayload & outPayload);

    /**
     * Populate the provided list of payloads.  This can handle concatenated payloads, and should
     * generally be preferred to populatePayload unless the payload is known to not be a
     * concatenated one.
     *
     * On success, the contents of outPayloads are replaced with the list of parsed payloads.
     *
     * On failure, nothing should be assumed about the state of outPayloads.
     */
    CHIP_ERROR populatePayloads(std::vector<SetupPayload> & outPayloads) const;

    // TODO: ExtractPayload may need to change significantly.
    // See https://github.com/project-chip/connectedhomeip/issues/38731
    static std::string ExtractPayload(std::string inString);

private:
    static CHIP_ERROR retrieveOptionalInfos(SetupPayload & outPayload, TLV::ContiguousBufferTLVReader & reader);
    static CHIP_ERROR populateTLV(SetupPayload & outPayload, const std::vector<uint8_t> & buf, size_t & index);
    static CHIP_ERROR parseTLVFields(chip::SetupPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes);

    static CHIP_ERROR populatePayloadFromBase38Data(std::string payload, SetupPayload & outPayload);
};

} // namespace chip
