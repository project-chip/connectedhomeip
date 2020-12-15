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

#include <core/CHIPError.h>
#include <core/CHIPTLV.h>

#include <string>
#include <utility>

namespace chip {

/**
 * @class AdditionalDataPayloadParser
 * A class that can be used to convert a base41 encoded payload to a AdditionalDataPayload object
 * */
class AdditionalDataPayloadParser
{
private:
    std::string mPayload;

public:
    AdditionalDataPayloadParser(std::string payload) : mPayload(std::move(payload)) {}
    CHIP_ERROR populatePayload(AdditionalDataPayload & outPayload);

};

} // namespace chip
