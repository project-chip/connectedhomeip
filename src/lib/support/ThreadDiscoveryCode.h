/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

namespace chip {
namespace Thread {

class DiscoveryCode
{
public:
    DiscoveryCode() : mCode(0) {}
    DiscoveryCode(uint16_t discriminator, uint32_t pincode, uint8_t version = 0) :
        DiscoveryCode(static_cast<uint8_t>(discriminator >> 8), pincode, version)
    {}
    DiscoveryCode(uint8_t discriminator, uint32_t pincode, uint8_t version = 0);
    uint64_t AsUInt64() const { return mCode; }
    std::vector<uint8_t> ToByteArray() const;
    bool IsAny() const { return mCode == 0; }

private:
    uint64_t mCode;
};

} // namespace Thread
} // namespace chip
