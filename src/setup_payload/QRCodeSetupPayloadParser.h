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

#include "SetupPayload.h"

#include <core/CHIPError.h>
#include <core/CHIPTLV.h>

#include <math.h>
#include <string.h>
#include <string>
#include <utility>

namespace chip {
std::string extractPayload(std::string inString);
/**
 * @class QRCodeSetupPayloadParser
 * A class that can be used to convert a base41 encoded payload to a SetupPayload object
 * */
class QRCodeSetupPayloadParser
{
private:
    std::string mBase41Representation;

public:
    QRCodeSetupPayloadParser(std::string base41Representation) : mBase41Representation(std::move(base41Representation)) {}
    CHIP_ERROR populatePayload(SetupPayload & outPayload);

private:
    CHIP_ERROR retrieveOptionalInfos(SetupPayload & outPayload, TLV::TLVReader & reader);
    CHIP_ERROR populateTLV(SetupPayload & outPayload, const std::vector<uint8_t> & buf, size_t & index);
    CHIP_ERROR parseTLVFields(chip::SetupPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes);
};

} // namespace chip
