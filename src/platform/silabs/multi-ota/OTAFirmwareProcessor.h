/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/Span.h>
#include <platform/silabs/multi-ota/OTATlvProcessor.h>

namespace chip {

class OTAFirmwareProcessor : public OTATlvProcessor
{
public:
    struct Descriptor
    {
        uint32_t version;
        char versionString[kVersionStringSize];
        char buildDate[kBuildDateSize];
    };

    CHIP_ERROR Init() override;
    CHIP_ERROR Clear() override;
    CHIP_ERROR ApplyAction() override;
    CHIP_ERROR FinalizeAction() override;

private:
    CHIP_ERROR ProcessInternal(ByteSpan & block) override;
    CHIP_ERROR ProcessDescriptor(ByteSpan & block);

    OTADataAccumulator mAccumulator;
    bool mDescriptorProcessed = false;
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
    uint32_t mUnalignmentNum;
#endif
    static constexpr size_t kAlignmentBytes = 64;
    static uint32_t mWriteOffset; // End of last written block
    static uint8_t mSlotId;       // Bootloader storage slot
    // Bootloader storage API requires the buffer size to be a multiple of 4.
    static uint8_t writeBuffer[kAlignmentBytes] __attribute__((aligned(4)));
    // Offset indicates how far the write buffer has been filled
    static uint16_t writeBufOffset;
};

} // namespace chip
