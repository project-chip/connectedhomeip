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

/**
 *    @file
 *      This file defines shared over-the-air protocol structures for
 *      Wi-Fi PAF (NAN/USD) commissioning.
 *
 */

#pragma once

#include <cstdint>

namespace chip {
namespace WiFiPAF {

// Service Specific Info (SSI) carried in NAN Publish frames.
#pragma pack(push, 1)
struct PAFPublishSSI
{
    uint8_t DevOpCode;
    uint16_t DevInfo; // Discriminator
    uint16_t VendorId;
    uint16_t ProductId;
};
#pragma pack(pop)

static_assert(sizeof(PAFPublishSSI) == 7, "PAFPublishSSI must be 7 bytes (no padding)");

} // namespace WiFiPAF
} // namespace chip
