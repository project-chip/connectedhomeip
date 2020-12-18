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
 *      This file describes a AdditionalData Payload parser based on the
 *      CHIP specification.
 */

#pragma once

#include "AdditionalDataPayload.h"

#include <stdint.h>
#include <core/CHIPError.h>

namespace chip {

/**
 * @class AdditionalDataPayloadParser
 * A class that can be used to convert a HEX encoded payload to a AdditionalDataPayload object
 * */
class AdditionalDataPayloadParser
{
private:
    const uint8_t * mPayloadBufferData;
    const uint32_t mPayloadBufferLength;


public:
    AdditionalDataPayloadParser(const uint8_t * payloadBufferData, const uint32_t payloadBufferLength) : mPayloadBufferData(payloadBufferData), mPayloadBufferLength(payloadBufferLength) {}
    CHIP_ERROR populatePayload(SetupPayload::AdditionalDataPayload & outPayload);
};

} // namespace chip
