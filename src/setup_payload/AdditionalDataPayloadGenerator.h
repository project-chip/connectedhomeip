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

#define ROTATING_DEVICE_ID_COUNTER_STR_LENGTH 10
#define ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH 16
#define ROTATING_DEVICE_ID_LENGTH ROTATING_DEVICE_ID_COUNTER_STR_LENGTH + ROTATING_DEVICE_ID_HASH_SUFFIX_LENGTH

namespace chip {

class AdditionalDataPayloadGenerator
{

public:
    AdditionalDataPayloadGenerator(uint16_t rotationCounter, char * serialNumberBuffer, size_t serialNumberBufferSize);

    // Generate additional data payload (i.e. TLV encoded)
    CHIP_ERROR generateAdditionalDataPayload(chip::System::PacketBufferHandle & bufferHandle);
    // Generate Device Rotating ID
    CHIP_ERROR generateRotatingDeviceId(char * rotatingDeviceIdBuffer, uint8_t & rotatingDeviceIdBufferSize);

private:
    const uint16_t mRotationCounter;
    const char * mSerialNumberBuffer;
    const size_t mSerialNumberBufferSize;
};

} // namespace chip
