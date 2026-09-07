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

#include "ThreadDiscoveryCode.h"

#include <stdint.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Thread {

namespace {
// Magic number "MT" in ASCII
constexpr uint32_t kMagicNumber = 0x4d540000;
} // namespace

DiscoveryCode::DiscoveryCode(uint16_t discriminator)
{
    // The discovery code packing:
    // Bits 32-63: Magic number "MT" (0x4D540000)
    // Bits 8-15: Lower 8 bits of the 12-bit discriminator
    // Bits 0-3: Upper 4 bits of the 12-bit discriminator

    uint64_t magic     = static_cast<uint64_t>(kMagicNumber) << 32;
    uint64_t discLow8  = static_cast<uint64_t>(discriminator & 0xFF) << 8;
    uint64_t discHigh4 = static_cast<uint64_t>((discriminator >> 8) & 0x0F);

    mCode = magic | discLow8 | discHigh4;
}

} // namespace Thread
} // namespace chip
