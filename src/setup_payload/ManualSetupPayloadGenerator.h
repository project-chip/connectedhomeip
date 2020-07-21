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
 *      into 3 chunks <32 bit><16 bit><16 bit>:
 *      - <32 bit> Represents:
 *          - <1 bit> 1 if vid+pid present, 0 otherwise
 *          - <27 bit> setup pin code
 *          - <4 bit> discriminator
 *      - <16 bit> Vendor ID
 *      - <16 bit> Product ID
 *
 */

#ifndef _MANUAL_SETUP_PAYLOAD_GENERATOR_H_
#define _MANUAL_SETUP_PAYLOAD_GENERATOR_H_

#include "SetupPayload.h"

#include <core/CHIPError.h>

#include <string>

using namespace std;
namespace chip {

class ManualSetupPayloadGenerator
{
private:
    SetupPayload mSetupPayload;

public:
    ManualSetupPayloadGenerator(const SetupPayload & payload) : mSetupPayload(payload){};

    // Populates decimal string representation of the payload into outDecimalString
    CHIP_ERROR payloadDecimalStringRepresentation(string & outDecimalString);
};

}; // namespace chip

#endif // _MANUAL_SETUP_PAYLOAD_GENERATOR_H_
