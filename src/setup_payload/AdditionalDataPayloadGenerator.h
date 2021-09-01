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
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace RotatingDeviceId {
static constexpr unsigned kLifetimeCounterSize = 2;
static constexpr unsigned kHashSuffixLength    = 16;
static constexpr unsigned kMaxLength           = kLifetimeCounterSize + kHashSuffixLength;
static constexpr unsigned kHexMaxLength        = kMaxLength * 2 + 1;
} // namespace RotatingDeviceId

enum class AdditionalDataFields : int8_t
{
    NotSpecified     = 0x00,
    RotatingDeviceId = 0x01
};

class AdditionalDataPayloadGenerator
{

public:
    AdditionalDataPayloadGenerator() {}

    /**
     * Generate additional data payload (i.e. TLV encoded).
     *
     * @param lifetimeCounter lifetime counter
     * @param serialNumberBuffer null-terminated serial number buffer
     * @param serialNumberBufferSize size of the serial number buffer supplied.
     * @param bufferHandle output buffer handle
     * @param additionalDataFields bitfield for what fields should be generated in the additional data
     *
     * @retval #CHIP_ERROR_INVALID_TLV_TAG
     *                              If the specified tag value is invalid or inappropriate in the context
     *                              in which the value is being written.
     * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
     *                              If writing the value would exceed the limit on the maximum number of
     *                              bytes specified when the writer was initialized.
     * @retval #CHIP_ERROR_NO_MEMORY
     *                              If an attempt to allocate an output buffer failed due to lack of
     *                              memory.
     * @retval other                Other CHIP or platform-specific errors returned by the configured
     *                              TLVBackingStore
     *
     */
    CHIP_ERROR generateAdditionalDataPayload(uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                             size_t serialNumberBufferSize, chip::System::PacketBufferHandle & bufferHandle,
                                             BitFlags<AdditionalDataFields> additionalDataFields);
    // Generate Device Rotating ID
    /**
     * Generate additional data payload (i.e. TLV encoded).
     *
     * @param lifetimeCounter lifetime counter
     * @param serialNumberBuffer null-terminated serial number buffer
     * @param serialNumberBufferSize size of the serial number buffer supplied.
     * @param rotatingDeviceIdBuffer rotating device id buffer
     * @param rotatingDeviceIdBufferSize the current size of the supplied buffer
     * @param rotatingDeviceIdValueOutputSize the number of chars making up the actual value of the returned rotating device id
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise.
     *
     */
    CHIP_ERROR generateRotatingDeviceId(uint16_t lifetimeCounter, const char * serialNumberBuffer, size_t serialNumberBufferSize,
                                        char * rotatingDeviceIdBuffer, size_t rotatingDeviceIdBufferSize,
                                        size_t & rotatingDeviceIdValueOutputSize);
};

} // namespace chip
