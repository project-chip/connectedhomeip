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
 *      This file describes a Manual Entry Code Generator based on the
 *      CHIP specification.
 *
 *      The encoding of the binary data to a decimal string is split
 *      into 5 chunks <1-digit/3-bits><5 digits/16-bits><4-digits/13-bits><5-digits/16-bits><5-digits/16-bits>:
 *      - <1 digit> Represents:
 *          - <bits 1..0> Discriminator <bits 11.10>
 *          - <bit 2> VID/PID present flag
 *      - <5 digits> Represents:
 *          - <bits 13..0> PIN Code <bits 13..0>
 *          - <bits 15..14> Discriminator <bits 9..8>
 *      - <4 digits> Represents:
 *          - <bits 12..0> PIN Code <bits 26..14>
 *      - <5 digits> Vendor ID
 *      - <5 digits> Product ID
 *
 */

#pragma once

#include "SetupPayload.h"

#include <lib/core/CHIPError.h>

#include <string>

namespace chip {

class ManualSetupPayloadGenerator
{
private:
    SetupPayload mSetupPayload;

public:
    ManualSetupPayloadGenerator(const SetupPayload & payload) : mSetupPayload(payload) {}

    // Populates decimal string representation of the payload into outDecimalString
    CHIP_ERROR payloadDecimalStringRepresentation(std::string & outDecimalString);
};

} // namespace chip
