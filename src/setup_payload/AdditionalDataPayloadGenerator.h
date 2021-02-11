/**
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

/**
 *    @file
 *      This file provides a utility to generate Additional Data payload and its members
 *      (e.g. rotating device id)
 *
 */

#pragma once
#include <core/CHIPError.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <type_traits>

namespace chip {
namespace RotatingDeviceId {
static constexpr unsigned kRotatingDeviceIdCounterStringMaxLength = 10;
static constexpr unsigned kRotatingDeviceIdHashSuffixLength       = 16;
static constexpr unsigned kRotatingDeviceIdMaxLength = kRotatingDeviceIdCounterStringMaxLength + kRotatingDeviceIdHashSuffixLength;
static constexpr unsigned kRotatingDeviceIdHexMaxLength = kRotatingDeviceIdMaxLength * 2 + 1;
} // namespace RotatingDeviceId

enum class AdditionalDataFields : int8_t
{
    NotSpecified     = 0x00,
    RotatingDeviceId = 0x01
};

inline AdditionalDataFields operator|(AdditionalDataFields lhs, AdditionalDataFields rhs)
{
    using T = std::underlying_type_t<AdditionalDataFields>;
    return static_cast<AdditionalDataFields>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline AdditionalDataFields & operator|=(AdditionalDataFields & lhs, AdditionalDataFields rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline AdditionalDataFields operator&(AdditionalDataFields lhs, AdditionalDataFields rhs)
{
    using T = std::underlying_type_t<AdditionalDataFields>;
    return static_cast<AdditionalDataFields>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

class AdditionalDataPayloadGenerator
{

public:
    AdditionalDataPayloadGenerator() {}

    /**
     * Generate additional data payload (i.e. TLV encoded).
     *
     * @param lifetimeCounter lifetime counter
     * @param serialNumberBuffer serial number buffer
     * @param serialNumberBufferSize size of the serial number buffer supplied.
     * @param bufferHandle output buffer handle
     * @param additionalDataFields bitfield for what fields should be generated in the additional data
     *
     * @return CHIP_NO_ERROR on success.
     *
     */
    CHIP_ERROR generateAdditionalDataPayload(uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                             size_t serialNumberBufferSize, chip::System::PacketBufferHandle & bufferHandle,
                                             AdditionalDataFields additionalDataFields);
    // Generate Device Rotating ID
    /**
     * Generate additional data payload (i.e. TLV encoded).
     *
     * @param lifetimeCounter lifetime counter
     * @param serialNumberBuffer serial number buffer
     * @param serialNumberBufferSize size of the serial number buffer supplied.
     * @param rotatingDeviceIdBuffer rotating device id buffer
     * @param rotatingDeviceIdBufferSize the current size of the supplied buffer
     * @param rotatingDeviceIdValueOutputSize the number of chars making up the actual value of the returned rotating device id
     *
     * @return CHIP_NO_ERROR on success.
     *
     */
    CHIP_ERROR generateRotatingDeviceId(uint16_t lifetimeCounter, const char * serialNumberBuffer, size_t serialNumberBufferSize,
                                        char * rotatingDeviceIdBuffer, size_t rotatingDeviceIdBufferSize,
                                        size_t & rotatingDeviceIdValueOutputSize);
};

} // namespace chip
