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

#if CHIP_HAVE_CONFIG_H
#include <setup_payload/CHIPAdditionalDataPayloadBuildConfig.h>
#endif

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

struct AdditionalDataPayloadGeneratorParams
{
#if CHIP_ENABLE_ROTATING_DEVICE_ID
    uint16_t rotatingDeviceIdLifetimeCounter;
    ByteSpan rotatingDeviceIdUniqueId;
#endif
};

class AdditionalDataPayloadGenerator
{

public:
    AdditionalDataPayloadGenerator() {}

    /**
     * Generate additional data payload (i.e. TLV encoded).
     *
     * @param params parameters needed to generate additional data payload
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
    CHIP_ERROR generateAdditionalDataPayload(AdditionalDataPayloadGeneratorParams & params,
                                             chip::System::PacketBufferHandle & bufferHandle,
                                             BitFlags<AdditionalDataFields> additionalDataFields);

#if CHIP_ENABLE_ROTATING_DEVICE_ID
    /**
     * Generate Rotating Device ID in Binary Format
     *
     * @param params parameters needed to generate additional data payload
     * @param [in,out] rotatingDeviceIdBuffer as input, the buffer to use for
     *                 the binary data.  As output, will have its size set to
     *                 the actual size used upon successful generation
     */
    CHIP_ERROR generateRotatingDeviceIdAsBinary(AdditionalDataPayloadGeneratorParams & params,
                                                MutableByteSpan & rotatingDeviceIdBuffer);

    /**
     * Generate Device Rotating ID in String Format
     *
     * @param params parameters needed to generate additional data payload
     * @param rotatingDeviceIdBuffer rotating device id buffer
     * @param rotatingDeviceIdBufferSize the current size of the supplied buffer
     * @param rotatingDeviceIdValueOutputSize the number of chars making up the actual value of the returned rotating device id
     * excluding the null terminator
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise.
     *
     */
    CHIP_ERROR generateRotatingDeviceIdAsHexString(AdditionalDataPayloadGeneratorParams & params, char * rotatingDeviceIdBuffer,
                                                   size_t rotatingDeviceIdBufferSize, size_t & rotatingDeviceIdValueOutputSize);
#endif
};

} // namespace chip
