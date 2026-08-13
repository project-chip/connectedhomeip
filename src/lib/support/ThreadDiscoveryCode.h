/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <stdlib.h>

namespace chip {
namespace Thread {

class DiscoveryCode
{
public:
    DiscoveryCode() : mCode(0) {}

    // This is initializing the DiscoveryCode with a 12-bit discriminator
    DiscoveryCode(uint16_t discriminator);

    // This is initializing the DiscoveryCode with a 4-bit short discriminator
    DiscoveryCode(uint8_t discriminator) : DiscoveryCode(static_cast<uint16_t>(discriminator << 8)) {}

    uint64_t AsUInt64Short() const { return (mCode & 0xffff'ffff'ffff'000fULL); }
    uint64_t AsUInt64() const { return mCode; }
    bool IsAny() const { return mCode == 0; }

private:
    uint64_t mCode;
};

} // namespace Thread
} // namespace chip
