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
 *      This file describes a Manual Setup Payload parser based on the
 *      CHIP specification.
 */

#pragma once

#include "SetupPayload.h"

#include <algorithm>
#include <lib/core/CHIPError.h>
#include <string>
#include <utility>

namespace chip {

/**
 * @class ManualSetupPayloadParser
 * A class that can be used to convert a decimal encoded payload to a SetupPayload object
 * */
class ManualSetupPayloadParser
{
private:
    std::string mDecimalStringRepresentation;

public:
    ManualSetupPayloadParser(std::string decimalRepresentation) : mDecimalStringRepresentation(std::move(decimalRepresentation))
    {
        // '-' might be being used in the decimal code as a digit group
        // separator, to aid in readability.  It's not actually part of the
        // decomal code, so strip it out.
        mDecimalStringRepresentation.erase(
            std::remove(mDecimalStringRepresentation.begin(), mDecimalStringRepresentation.end(), '-'),
            mDecimalStringRepresentation.end());
    }
    CHIP_ERROR populatePayload(SetupPayload & outPayload);

    static CHIP_ERROR CheckDecimalStringValidity(std::string decimalString, std::string & decimalStringWithoutCheckDigit);
    static CHIP_ERROR CheckCodeLengthValidity(const std::string & decimalString, bool isLongCode);
    static CHIP_ERROR ToNumber(const std::string & decimalString, uint32_t & dest);
    static CHIP_ERROR ReadDigitsFromDecimalString(const std::string & decimalString, size_t & index, uint32_t & dest,
                                                  size_t numberOfCharsToRead);
};

} // namespace chip
